#include <algorithm>
#include <limits>
#include <numeric>
#include <cmath>
#include <HashColon/Statistics/SimpleStatisticalAnalysis.hpp>
#include <HashColon/Clustering/ClusterEvaluation.hpp>

using namespace std;
using namespace Eigen;
using namespace HashColon;
using namespace HashColon::Statistics;

namespace HashColon::Clustering
{	
	// Get Min/Max/Mean/Median/Standard deviation from a given cluster
	SimpleStatisticsAnalysisResults DistanceAnalysis(const vector<Real>& DistancesInCluster)
	{
		SimpleStatisticsAnalysisResults re;	
		re.NewAll();		
		/*(*re.Min) = Min(DistancesInCluster);
		(*re.Max) = Max(DistancesInCluster);
		(*re.Mean) = Mean(DistancesInCluster);
		(*re.Median) = Median(DistancesInCluster);
		(*re.Variance) = Variance(DistancesInCluster);
		(*re.StdDeviation) = StdDeviation(DistancesInCluster);*/

		return re;
	}

	DistancesAnalysisResults DistanceAnalysis(const vector<vector<Real>>& DistancesInClusters)
	{
		DistancesAnalysisResults re;
		vector<Real> wholeDist;
		re.ClusterResults.resize(DistancesInClusters.size());
		for (size_t i = 0; i < DistancesInClusters.size(); i++)
		{
			re.ClusterResults[i] = DistanceAnalysis(DistancesInClusters[i]);
			copy(DistancesInClusters[i].begin(), DistancesInClusters[i].end(), back_inserter(wholeDist));
		}
		re.TotalResults = DistanceAnalysis(wholeDist);
		return re;
	}

	vector<vector<Real>> SortedDistanceGraph(
		const vector<size_t>& clusterResult, const MatrixXR& DistanceMatrix)
	{
		// DistanceMatrix should be square matrix & size of cluster result should be equal to size of distance matrix
		assert((int)clusterResult.size() == DistanceMatrix.rows());
		assert((int)DistanceMatrix.rows() == DistanceMatrix.cols());
		const auto N = clusterResult.size();

		// count number of clusters
		size_t clusterNo = (*max_element(clusterResult.begin(), clusterResult.end())) + 1;

		// Distances grouped by each cluster
		vector<vector<Real>> DistancesInClusters;
		DistancesInClusters.resize(clusterNo);
		for (size_t i = 0; i < N; i++)
		{
			for (size_t j = i + 1; j < N; j++)
			{
				if (clusterResult[i] == clusterResult[j])
				{
					DistancesInClusters[clusterResult[i]].push_back(DistanceMatrix(i, j));
				}
			}
		}

		// sort results
		for (auto& distances : DistancesInClusters)
		{
			sort(distances.begin(), distances.end());
		}

		return DistancesInClusters;
	}

	vector<vector<Real>> SortedDistanceGraph(
		const vector<size_t>& clusterResult, const MatrixXR& DistanceMatrix,
		DistancesAnalysisResults& additionals)
	{
		// DistanceMatrix should be square matrix & size of cluster result should be equal to size of distance matrix
		assert(clusterResult.size() == DistanceMatrix.rows());
		assert(DistanceMatrix.rows() == DistanceMatrix.cols());
		const auto N = clusterResult.size();

		// count number of clusters
		size_t clusterNo = (*max_element(clusterResult.begin(), clusterResult.end())) + 1;

		// Distances grouped by each cluster
		vector<vector<Real>> DistancesInClusters; 				
		DistancesInClusters.resize(clusterNo);
		for (size_t i = 0; i < N; i++)
		{
			for (size_t j = i + 1; j < N; j++)
			{				
				if (clusterResult[i] == clusterResult[j])
				{
					DistancesInClusters[clusterResult[i]].push_back(DistanceMatrix(i, j));					
				}				
			}			
		}

		// sort results
		for (auto& distances : DistancesInClusters)
		{
			sort(distances.begin(), distances.end());
		}

		// compute distance analysis
		additionals = DistanceAnalysis(DistancesInClusters);		

		return DistancesInClusters;
	}

	vector<size_t> PseudoMedian(
		const std::vector<size_t>& clusterResult, const Eigen::MatrixXR& DistanceMatrix)
	{
		// DistanceMatrix should be square matrix & size of cluster result should be equal to size of distance matrix
		assert((int)clusterResult.size() == DistanceMatrix.rows());
		assert(DistanceMatrix.rows() == DistanceMatrix.cols());
		const auto N = clusterResult.size();

		size_t clusterNo = (*max_element(clusterResult.begin(), clusterResult.end())) + 1;
		vector<size_t> medians(clusterNo);
		vector<Real> sum(clusterNo, numeric_limits<Real>::max());

		for (size_t i = 0; i < N; i++)
		{			
			Real tmpsum = 0;
			for (size_t j = 0; j < N; j++)
			{
				if (i == j) continue;
				else if (clusterResult[i] == clusterResult[j])
				{
					tmpsum += DistanceMatrix((int)i, (int)j);
				}
			}
			if (tmpsum < sum[clusterResult[i]])
			{
				sum[clusterResult[i]] = tmpsum;
				medians[clusterResult[i]] = i;
			}				
		}

		return medians;
	}

	std::vector<HashColon::Real> PseudoDaviesBouldin(
		const std::vector<size_t>& clusterResult, const Eigen::MatrixXR& DistanceMatrix,
		std::vector<size_t>& pseudoMedian)
	{
		// DistanceMatrix should be square matrix & size of cluster result should be equal to size of distance matrix
		assert((int)clusterResult.size() == DistanceMatrix.rows());
		assert(DistanceMatrix.rows() == DistanceMatrix.cols());
		const auto N = clusterResult.size();

		size_t clusterNo = (*max_element(clusterResult.begin(), clusterResult.end())) + 1;		
		vector<Real> re(clusterNo, 0.0);
		vector<Real> cnt(clusterNo, 0.0);

		pseudoMedian = PseudoMedian(clusterResult, DistanceMatrix);

		for (size_t i = 0; i < N; i++)
		{			
			Real d = DistanceMatrix(i, pseudoMedian[clusterResult[i]]);
			re[clusterResult[i]] += d * d;
			cnt[clusterResult[i]] += 1.0;
		}

		for (size_t i = 0 ; i < clusterNo; i++)
		{
			if (cnt[i] == 0)
				re[i] = 0;
			else 
				re[i] = sqrt(re[i] / cnt[i]);
		}

		return re;		
	}

	std::vector<HashColon::Real> PseudoDaviesBouldin(
		const std::vector<size_t>& clusterResult, const Eigen::MatrixXR& DistanceMatrix)
	{
		std::vector<size_t> pseudoMedian;
		return PseudoDaviesBouldin(clusterResult, DistanceMatrix, pseudoMedian);
	}

	HashColon::Real Silhouette(size_t itemIdx, const std::vector<size_t>& clusterResult, const Eigen::MatrixXR& DistanceMatrix)
	{
		// DistanceMatrix should be square matrix & size of cluster result should be equal to size of distance matrix
		assert(clusterResult.size() == DistanceMatrix.rows());
		assert(DistanceMatrix.rows() == DistanceMatrix.cols());
		const auto N = clusterResult.size();

		// compute fitness to each clusters
		size_t clusterNo = (*max_element(clusterResult.begin(), clusterResult.end())) + 1;
		vector<Real> A(clusterNo, 0.0);
		vector<Real> cnt(clusterNo, 0.0);
		for (size_t i = 0; i < N; i++)
		{
			if (i == itemIdx) continue;
			A[clusterResult[i]] += DistanceMatrix(itemIdx, i);
			cnt[clusterResult[i]] += 1.0;
		}

		// if cnt of the cluster including itemIdx is equal or lesser than 1.0, return 0.0;
		if (cnt[clusterResult[itemIdx]] <= 1.0) return 0.0;

		// compute a(itemIdx) && b(itemIdx)
		Real a_item = 0.0;
		Real b_item = numeric_limits<Real>::max();
		for (size_t i = 0; i < A.size(); i++)
		{
			Real tmpVal = A[i] / (cnt[i]);
			if (i == clusterResult[itemIdx])
			{
				a_item = tmpVal;
			}
			else if(tmpVal < b_item)
			{
				b_item = tmpVal;
			}			
		}

		Real div = a_item > b_item ? a_item : b_item;
		return (b_item - a_item) / div;
	}

	std::vector<HashColon::Real> Silhouette(const std::vector<size_t>& clusterResult, const Eigen::MatrixXR& DistanceMatrix)
	{
		// DistanceMatrix should be square matrix & size of cluster result should be equal to size of distance matrix
		assert(clusterResult.size() == DistanceMatrix.rows());
		assert(DistanceMatrix.rows() == DistanceMatrix.cols());
		const auto N = clusterResult.size();

		vector<Real> re(N);

		#pragma omp parallel for
		for (size_t i = 0; i < N; i++)
		{
			re[i] = Silhouette(i, clusterResult, DistanceMatrix);
		}

		return re;
	}
}