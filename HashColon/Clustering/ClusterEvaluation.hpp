#ifndef HASHCOLON_CLUSTERING_CLUSTEREVALUATION_HPP
#define HASHCOLON_CLUSTERING_CLUSTEREVALUATION_HPP

#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif

#include <vector>
#include <memory>
#include <Eigen/Eigen>
#include <HashColon/Core/Real.hpp>
#include <HashColon/Statistics/SimpleStatisticalAnalysis.hpp>

namespace  HashColon::Clustering
{	
	struct DistancesAnalysisResults
	{
		std::vector<HashColon::Statistics::SimpleStatisticsAnalysisResults> ClusterResults;
		HashColon::Statistics::SimpleStatisticsAnalysisResults TotalResults;
	};

	HashColon::Statistics::SimpleStatisticsAnalysisResults DistanceAnalysis(const std::vector<HashColon::Real>& DistancesInCluster);

	DistancesAnalysisResults DistanceAnalysis(
		const std::vector<size_t>& clusterResult, const Eigen::MatrixXR& DistanceMatrix);

	std::vector<std::vector<HashColon::Real>> SortedDistanceGraph(
		const std::vector<size_t>& clusterResult, const Eigen::MatrixXR& DistanceMatrix);

	std::vector<std::vector<HashColon::Real>> SortedDistanceGraph(
		const std::vector<size_t>& clusterResult, const Eigen::MatrixXR& DistanceMatrix,
		DistancesAnalysisResults& additionals);

	std::vector<size_t> PseudoMedian(
		const std::vector<size_t>& clusterResult, const Eigen::MatrixXR& DistanceMatrix);
	
	/* 
	* The Original definition of Davies-Bouldin index of a cluster is vary similar to standard deviation of a clsuter
	* 
	* DBI_i = sqrt ( 1/N_i ( sum( ( X_k - C_i ) ^ 2 ) )
	*/
	std::vector<HashColon::Real> PseudoDaviesBouldin(
		const std::vector<size_t>& clusterResult, const Eigen::MatrixXR& DistanceMatrix);

	std::vector<HashColon::Real> PseudoDaviesBouldin(
		const std::vector<size_t>& clusterResult, const Eigen::MatrixXR& DistanceMatrix,
		std::vector<size_t>& pseudoMedian);
		
	// silhouette for single data
	HashColon::Real Silhouette(size_t itemIdx, const std::vector<size_t>& clusterResult, const Eigen::MatrixXR& DistanceMatrix);

	// silhouette for all data
	std::vector<HashColon::Real> Silhouette(const std::vector<size_t>& clusterResult, const Eigen::MatrixXR& DistanceMatrix);
	//HashColon::Real ClassificationError(const std::vector<size_t>& clusterResult, const std::vector<size_t>& givenLabel);
	//HashColon::Real VariationOfInfomration(const std::vector<size_t>& clusterResult, const std::vector<size_t>& givenLabel);
}

#endif