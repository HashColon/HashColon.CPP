#ifndef HASHCOLON_CLUSTERING_DBSCAN_IMPL
#define HASHCOLON_CLUSTERING_DBSCAN_IMPL

#include <HashColon/Clustering/DBSCAN.hpp>
#include <queue>
#include <sstream>
#include <boost/type_index.hpp>
#include <HashColon/Core/CommonLogger.hpp>
#include <HashColon/Helper/StringHelper.hpp>

namespace HashColon::Clustering
{
	template<typename T>
	void DistanceBasedDBSCAN<T>::Initialize(
		const std::string identifierPostfix,
		const std::string configFilePath)
	{
		using namespace std;
		using namespace HashColon;
		using namespace HashColon::Helper;
		using namespace boost::typeindex;

		string identifier = "Clustering.DistanceBasedDBSCAN";
		if (identifierPostfix.empty())
		{
			identifier += ("_" + Split(type_id<T>().pretty_name(), ":").back());
		}
		else
		{
			identifier = identifier + "_" + identifierPostfix;
		}

		CLI::App* cli = HashColon::SingletonCLI::GetInstance().GetCLI(identifier);

		if (!configFilePath.empty())
		{
			HashColon::SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--minPts", _cDefault.minPts, "minPts value. If a point is a core point, at least minPts number of points should be in range of epsilon.");
		cli->add_option("--DbscanEpsilon", _cDefault.DbscanEpsilon, "Epsilon value. Range value for cheking density");
		cli->add_option("--Verbose", _cDefault.Verbose, "Screen prints progress.");
	}

	

	template <typename T>
	void DistanceBasedDBSCAN<T>::TrainModel(
		const Eigen::MatrixXR& iRawDistMatrix, bool isDistance,
		typename ClusteringBase<T>::LabelsPtr oLabels,
		typename ClusteringBase<T>::ProbListPtr oProbabilities)
	{
		using namespace std;
		using namespace HashColon;
		using Tag = HashColon::LogUtils::Tag;

		CommonLogger logger;
		
		// Assertion	
		// assert at least 1 data is given for clustering
		assert(_c.minPts > 0);
		assert(iRawDistMatrix.cols() == iRawDistMatrix.rows());
		assert(iRawDistMatrix.cols() > _c.minPts);
		assert(!ClusteringBase<T>::isTrained);
		assert(oLabels != nullptr);
		if (ClusteringBase<T>::isTrained) throw Exception("Already trained.");
		if (oLabels == nullptr) throw Exception(this->GetMethodName() + " needs cluster label output for input.");

		{
			lock_guard<mutex> _lg(CommonLogger::_mutex);			
			logger.Debug({ {__CODEINFO_TAGS__} }) << "\n" <<
				"Raw distance matrix:\n" << iRawDistMatrix << endl;
		}
				
		// if distance measure is similarity type, convert to distance		
		Eigen::MatrixXR B = isDistance ? iRawDistMatrix	: ConvertSimilarity2Distance(iRawDistMatrix);		
		if (!isDistance)
		{
			lock_guard<mutex> _lg(CommonLogger::_mutex);
			logger.Log({ { Tag::lvl, 3 } }) << this->GetMethodName() << ": Raw distance matrix conversion from similarity to distance is finished. " << endl;
			logger.Debug({ {__CODEINFO_TAGS__} }) << "\n" <<
				"Converted distance matrix:\n" << B << endl;
		}

		// compute neighbors
		vector<vector<size_t>> neighbors = GetNeighbors(B);
		{
			lock_guard<mutex> _lg(CommonLogger::_mutex);
			logger.Log({ { Tag::lvl, 3 } }) << this->GetMethodName() << ": Neighgor computation is finished." << endl;

			stringstream ss;
			for (const auto& debugout1 : neighbors) 
			{
				for (const auto& debugout2 : debugout1)
				{
					ss << debugout2 << "\t";
				}
				ss << "\n";
			}
			logger.Debug({ {__CODEINFO_TAGS__} }) << "\n" <<
				"Neighbor lists:\n" << ss.str() << endl;
		}

		// Initialize cluster state
		oLabels->clear();
		oLabels->resize(iRawDistMatrix.cols());		

		// set initial clustering idx as 2
		// unclassified: 0, noise: 1, clustered: 2~		
		size_t clusterIdx = 2;
				
		// DBSCAN algorithm: for each data points
		for (size_t i = 0 ; i < (size_t)iRawDistMatrix.cols(); i++)
		{
			// if the point is classfied already, continue;
			if (oLabels->at(i) != unclassified) continue;

			// if the point is core point: has more then minPts in radius epsilon,
			// run bfs
			if (neighbors[i].size() >= _c.minPts)
			{
				DbscanBfs(i, clusterIdx, neighbors, (*oLabels));
				{
					lock_guard<mutex> _lg(CommonLogger::_mutex);
					size_t clustersize =count_if(oLabels->begin(), oLabels->end(), 
						[&clusterIdx](auto e) { return e == clusterIdx; });
					logger.Debug({ {__CODEINFO_TAGS__} }) << "Cluster " << clusterIdx << " built (" 
						<< clustersize << " items)" << endl;
				}
				clusterIdx++;
			}
			// else this point is noise
			else
			{
				oLabels->at(i) = noise;
				{
					lock_guard<mutex> _lg(CommonLogger::_mutex);
					size_t noisesize = count_if(oLabels->begin(), oLabels->end(),
						[](auto e) { return e == 1; });
					logger.Debug({ {__CODEINFO_TAGS__} }) << "Noise item detected. (" 
						<< noisesize << " items)" << endl;
				}
			}
		}

		// as all classification is finished,
		// remove unclassified flags 
		for (size_t i = 0; i < oLabels->size(); i++)
		{
			assert(oLabels->at(i) > 0);
			oLabels->at(i)--;
		}

		_numOfClusters = clusterIdx - 1;
		ClusteringBase<T>::isTrained = true;

		{
			lock_guard<mutex> _lg(CommonLogger::_mutex);
			logger.Log({ { Tag::lvl, 3} }) << this->GetMethodName() << ": Finished. (" << (_numOfClusters-1) << " clusters + noise)" << endl;
			stringstream ss;
			for (size_t i = 0; i < _numOfClusters; i++)
			{
				size_t itemCnt = count_if(oLabels->begin(), oLabels->end(),
					[&i](auto e) { return e == i; });
				if(i == 0)
					ss << "Noise\t: " << itemCnt << " items\n";
				else 
					ss << "C" << (i-1) << "\t: " << itemCnt << " items\n";
			}
			logger.Log({ {Tag::lvl, 3} }) << "\n" << ss.str() << flush;
		}		
	}

	template <typename T>
	std::vector<std::vector<size_t>> DistanceBasedDBSCAN<T>::GetNeighbors(const Eigen::MatrixXR& DistMatrix) const
	{
		using namespace std;
		assert(DistMatrix.cols() == DistMatrix.rows());
		vector<vector<size_t>> re; re.resize(DistMatrix.cols());

		#pragma omp parallel for
		for (size_t i = 0; i < (size_t)DistMatrix.cols(); i++)
			for (size_t j = 0; j < (size_t)DistMatrix.rows(); j++)
			{
				if (i == j)
					continue;
				else if (DistMatrix(i, j) < _c.DbscanEpsilon)
					re[i].push_back(j);
			}

		return re;
	}

	template <typename T>
	void DistanceBasedDBSCAN<T>::DbscanBfs(
		size_t initP, size_t clusterIdx,
		std::vector<std::vector<size_t>>& neighbors, std::vector<size_t>& labels) const
	{
		using namespace std;
		assert(neighbors[initP].size() >= _c.minPts);
		assert(labels[initP] == unclassified);
		queue<size_t> q; q.push(initP);

		do
		{
			// pop a point from queue, add it to current cluster
			size_t p = q.front(); q.pop();
			labels[p] = clusterIdx;

			// if current point has sufficient neighbors,
			if (neighbors[p].size() >= _c.minPts)
			{
				// push neighbors to queue 
				for (auto& n : neighbors[p]) {
					if(labels[n] != clusterIdx)
						q.push(n);
				}
			}
		} while (!q.empty());
	}
}

#endif
