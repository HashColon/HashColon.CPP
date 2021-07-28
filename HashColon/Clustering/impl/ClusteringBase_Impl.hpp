#ifndef HASHCOLON_CLUSTERING_CLUSTERINGBASE_IMPL
#define HASHCOLON_CLUSTERING_CLUSTERINGBASE_IMPL

#include <atomic>
#include <mutex>
#include <cassert>
#include <HashColon/Core/LogUtils.hpp>
#include <HashColon/Core/CommonLogger.hpp>
#include <HashColon/Clustering/ClusteringBase.hpp>

namespace HashColon::Clustering
{
	template<typename T>
	inline Eigen::MatrixXR DistanceBasedClustering<T>::ComputeDistanceMatrix(
		const typename ClusteringBase<T>::DataListType& iTrainingData,
		bool verbose) const
	{
		using namespace std;
		using namespace Eigen;
		using namespace HashColon;
		using namespace HashColon::LogUtils;

		// variables for progress verbose
		atomic<int> progressCnt{ 0 };
		mutex _m;
		HashColon::CommonLogger logger;

		size_t l = iTrainingData.size();
		MatrixXR re(l, l);

		struct ijtype { size_t i; size_t j; };
		vector<ijtype> a;
		for (size_t i = 0; i < l; i++)
			for (size_t j = i + 1; j < l; j++)
			{
				ijtype tmp; tmp.i = i; tmp.j = j;
				a.push_back(tmp);
			}

		#pragma omp parallel for 
		for (size_t c = 0; c < a.size(); c++)
		{
			size_t& i = a[c].i; size_t& j = a[c].j;
			re(i, j) = re(j, i) = MeasureFunc->Measure(iTrainingData[i], iTrainingData[j]);
			assert(!isnan(re(i, j)));
			assert(re(i, j) >= 0);

			// show progress
			if (verbose)
			{
				lock_guard<mutex> _lg(_m);
				stringstream tempss;
				tempss << "Computing distances: " << progressCnt << "/" << a.size() << " " << Frag::Percentage(++progressCnt, (int)a.size());
				logger.Message << Flashl(tempss.str());
				//logger.Message << tempss.str() << endl;
			}
		}
		return re;
	}

	template<typename T>
	void DistanceBasedClustering<T>::TrainModel(
		const typename ClusteringBase<T>::DataListType& iTrainingData,
		typename ClusteringBase<T>::LabelsPtr oLabels,
		typename ClusteringBase<T>::ProbListPtr oProbabilities)
	{
		using namespace std;
		using namespace HashColon;
		using Tag = HashColon::LogUtils::Tag;

		CommonLogger logger;
		{
			lock_guard<mutex> _lg(CommonLogger::_mutex);
			logger.Log({ { Tag::lvl, 3} }) << this->GetMethodName() << ": Started. Using " << this->MeasureFunc->GetMethodName() << endl;
		}

		// assertions
		assert(iTrainingData.size() >= 0);
		assert(!ClusteringBase<T>::isTrained);
		assert(oLabels);
		if (ClusteringBase<T>::isTrained) throw Exception("Already trained.");
		if (oLabels == nullptr) throw Exception(this->GetMethodName() + " needs cluster label output for input.");

		// compute raw distance matrix
		Eigen::MatrixXR RawDistMatrix = DistanceBasedClustering<T>::ComputeDistanceMatrix(iTrainingData);
		{
			lock_guard<mutex> _lg(CommonLogger::_mutex);
			logger.Log({ { Tag::lvl, 3 } }) << this->GetMethodName() << ": Raw distance computation finished. " << endl;			
		}
				
		bool isDistance = this->MeasureFunc->GetMeasureType() == DistanceMeasureType::distance;

		// run clustering 
		TrainModel(RawDistMatrix, isDistance, oLabels, oProbabilities);
	}
}

#endif	
