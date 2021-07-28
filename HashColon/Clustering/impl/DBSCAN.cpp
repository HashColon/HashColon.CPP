#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif

#include <vector>
#include <algorithm>
#include <queue>
#include <cassert>
#include <Eigen/Eigen>
#include <flann/flann.hpp>

#include <CLI11_modified/CLI11_extended.hpp>
#include <HashColon/Core/Exception.hpp>
#include <HashColon/Core/Real.hpp>
#include <HashColon/Core/SingletonCLI.hpp>
#include <HashColon/Core/CommonLogger.hpp>
#include <HashColon/Clustering/ClusteringBase.hpp>
#include <HashColon/Clustering/DBSCAN.hpp>

using namespace std;
using namespace Eigen;
using namespace HashColon;
using namespace HashColon::Clustering;
using Tag = HashColon::LogUtils::Tag;

using PointType = std::vector<HashColon::Real>;

void DBSCAN::Initialize(const string configFilePath)
{
	CLI::App* cli = SingletonCLI::GetInstance().GetCLI("Clustering.DBSCAN");

	if (!configFilePath.empty())
	{
		SingletonCLI::GetInstance().AddConfigFile(configFilePath);
	}

	cli->add_option("--minPts", _cDefault.minPts, "minPts value. If a point is a core point, at least minPts number of points should be in range of epsilon.");
	cli->add_option("--DbscanEpsilon", _cDefault.DbscanEpsilon, "Epsilon value. Range value for cheking density");
}

void DBSCAN::TrainModel(
	const typename ClusteringBase<PointType>::DataListType& iTrainingData,
	typename ClusteringBase<PointType>::LabelsPtr oLabels,
	typename ClusteringBase<PointType>::ProbListPtr oProbabilities)
{
	CommonLogger logger;
	{
		lock_guard<mutex> _lg(CommonLogger::_mutex);
		logger.Log({ { Tag::lvl, 3} }) << "DBSCAN: Started. " << endl;
	}

	// Assertion	
	// assert at least 1 data is given for clustering
	assert(_c.minPts > 0);
	assert(iTrainingData.size() >= _c.minPts);
	assert(!ClusteringBase<PointType>::isTrained);
	assert(oLabels != nullptr);
	if (ClusteringBase<PointType>::isTrained) throw Exception("Already trained.");
	if (oLabels == nullptr) throw Exception("DBSCAN needs cluster label output for input.");

	// compute distance matrix & neighbors
	vector<vector<size_t>> neighbors = GetNeighbors(iTrainingData);

	{
		lock_guard<mutex> _lg(CommonLogger::_mutex);
		logger.Log({ { Tag::lvl, 3 } }) << "DBSCAN: Neighbor computation finished. " << endl;
		
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
	oLabels->resize(iTrainingData.size());

	// set meanings for the cluster idx
	// unclassified: 0, noise: 1, clustered: 2~		
	size_t clusterIdx = 2;

	for (size_t i = 0; i < iTrainingData.size(); i++)
	{
		// if the point is classfied already, continue;
		if (oLabels->at(i) != unclassified) continue;

		// if the point is core point: has more then minPts in radius epsilon,
		// run bfs
		if (neighbors[i].size() >= _c.minPts)
		{
			DbscanBfs(i, clusterIdx, neighbors, (*oLabels));
			clusterIdx++;
		}
		// else this point is noise
		else
		{
			oLabels->at(i) = noise;
		}
	}

	// as all classification is finished,
		// remove unclassified flags 
	for (size_t i = 0; i < iTrainingData.size(); i++)
	{
		assert(oLabels->at(i) > 0);
		oLabels->at(i)--;
	}

	{
		lock_guard<mutex> _lg(CommonLogger::_mutex);
		logger.Log({ { Tag::lvl, 3} }) << "DBSCAN: Finished. " << endl;
	}
	_numOfClusters = clusterIdx;
	ClusteringBase<PointType>::isTrained = true;
}

vector<vector<size_t>> DBSCAN::GetNeighbors(
	const typename ClusteringBase<vector<Real>>::DataListType& iTrainingData) const
{
	vector<vector<size_t>> re; 

	// use flann to build neighbor information
	size_t dataN = iTrainingData.size();
	size_t dataD = iTrainingData[0].size();
	vector<Real> flannDataset_; flannDataset_.resize(dataN * dataD);
	flann::Matrix<Real> flannDataset(flannDataset_.data(), dataN, dataD);
	
	// copy sample data to flann Matrix
	#pragma omp parallel for
	for (size_t i = 0; i < dataN; i++)
	{
		for (size_t j = 0; j < dataD; j++)
			flannDataset[i][j] = iTrainingData[i][j];
	}
	// build kd-tree using flann using 2-norm
	flann::Index<flann::L2<double>> kdtree(flannDataset, flann::KDTreeIndexParams());
	kdtree.buildIndex();

	// results from flann
	vector<vector<int>> idxs;
	vector<vector<double>> dists;

	kdtree.radiusSearch(
		flannDataset,
		idxs, dists,
		(float)_c.DbscanEpsilon,
		flann::SearchParams()
	);

	// we don't need distance info 
	dists.clear();

	// convert vectr<vector<int>> to vectr<vector<size_t>>
	re.resize(idxs.size());
	#pragma omp parallel for
	for (size_t i = 0; i < idxs.size(); i++)
	{		
		re[i].resize(idxs[i].size());
		for (size_t j = 0; j < idxs[i].size(); i++)
		{
			re[i][j] = idxs[i][j];
		}
	}

	return re;
}

void DBSCAN::DbscanBfs(size_t initP, size_t clusterIdx,
	vector<vector<size_t>>& neighbors, vector<size_t>& labels) const
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
				q.push(n);
			}
		}
	} while (!q.empty());
}
