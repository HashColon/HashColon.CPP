#ifndef HASHCOLON_CLUSTERING_CLUSTERINGBASE_HPP
#define HASHCOLON_CLUSTERING_CLUSTERINGBASE_HPP

#include <array>
#include <vector>
#include <memory>
#include <variant>
#include <utility>
#include <Eigen/Eigen>
#include <HashColon/Helper/Real.hpp>

namespace HashColon::Clustering
{
	template<typename DataType>
	class ClusteringBase
	{
		// class built-in types definition
	public:
		using Ptr = std::shared_ptr<ClusteringBase<DataType>>;
		using DataListType = std::vector<DataType>;
		using LabelsType = std::vector<std::size_t>;
		using LabelsPtr = std::shared_ptr<std::vector<std::size_t>>;
		using ProbType = std::vector<HashColon::Helper::Real>;
		using ProbPtr = std::shared_ptr< std::vector<HashColon::Helper::Real>>;
		using ProbListType = std::vector<std::vector<HashColon::Helper::Real>>;
		using ProbListPtr = std::shared_ptr<std::vector<std::vector<HashColon::Helper::Real>>>;

		// trains clustering model with given training data		
		virtual void TrainModel(
			const DataListType& iTrainingData,
			LabelsPtr oLabels = nullptr,
			ProbListPtr oProbabilities = nullptr)
			= 0;

		// get cluster label for a given data sample.
		// training most be done before using this function.
		virtual size_t GetClusterOf(
			const DataType& iTestValue,
			ProbPtr oProbabilities = nullptr)
			= 0;

		// get number of cluster of the trained model.		
		virtual int GetNumOfClusters() = 0;

		// erase trained model.
		virtual void cleanup() = 0;

		// get clustering method name in string
		virtual const std::string GetMethodName() const = 0;

		// check if this clustering model is done. 
		// if TrainModel is done, this value is true. else, false.
		bool IsTrained() const { return isTrained; };

	protected:
		bool isTrained = false;
		ClusteringBase() : isTrained(false) {};
	};


	enum DistanceMeasureType { distance, similarity };

	template<typename DataType>
	class DistanceMeasureBase
	{
	public:
		using Ptr = std::shared_ptr<DistanceMeasureBase<DataType>>;
		virtual HashColon::Helper::Real Measure(const DataType& a, const DataType& b) const = 0;
		const DistanceMeasureType _measureType;
		const DistanceMeasureType GetMeasureType() const { return _measureType; };
		virtual const std::string GetMethodName() const = 0;
	protected:
		DistanceMeasureBase(DistanceMeasureType type) : _measureType(type) {};
	};


	template<typename DataType>
	class DistanceBasedClustering : public ClusteringBase<DataType>
	{	
	protected:
		typename DistanceMeasureBase<DataType>::Ptr MeasureFunc;
	public:
		typename DistanceMeasureBase<DataType>::Ptr GetDistanceFunc() { return MeasureFunc; };
		typename DistanceMeasureBase<DataType>::Ptr GetSimilarityFunc() { return MeasureFunc; };
	protected:
		DistanceBasedClustering(typename DistanceMeasureBase<DataType>::Ptr func)
			: MeasureFunc(func) {};

		Eigen::MatrixXR ComputeDistanceMatrix(
			const typename ClusteringBase<DataType>::DataListType& iTrainingData) const;
	};

	template <typename DataType>
	using SimilarityBasedClustering = DistanceBasedClustering<DataType>;

	template<typename T>
	inline Eigen::MatrixXR DistanceBasedClustering<T>::ComputeDistanceMatrix(
		const typename ClusteringBase<T>::DataListType& iTrainingData) const
	{
		using namespace std;
		using namespace Eigen;

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

		//#pragma omp parallel for 
		for (size_t c = 0; c < a.size(); c++)
		{
			size_t& i = a[c].i; size_t& j = a[c].j;
			re(i, j) = re(j, i) = MeasureFunc->Measure(iTrainingData[i], iTrainingData[j]);			
		}
		return re;
	}

}

#endif
