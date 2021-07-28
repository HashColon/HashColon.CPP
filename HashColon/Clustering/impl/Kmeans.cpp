#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif

#include <vector>
#include <Eigen/Eigen>

#include <dkm_modified/dkm_parallel.hpp>

#include <HashColon/Core/ext/CLI11/CLI11_extended.hpp>
#include <HashColon/Core/Exception.hpp>
#include <HashColon/Core/Real.hpp>
#include <HashColon/Core/SingletonCLI.hpp>
#include <HashColon/Core/CommonLogger.hpp>
#include <HashColon/Clustering/ClusteringBase.hpp>
#include <HashColon/Clustering/Kmeans.hpp>

using namespace std;
using namespace Eigen;
using namespace HashColon;
using namespace HashColon::Clustering;
using SingletonCLI = HashColon::SingletonCLI;
using Tag = HashColon::LogUtils::Tag;
using GlobalLogger = HashColon::CommonLogger;


using PointType = std::vector<HashColon::Real>;

void Kmeans::Initialize(const string configFilePath)
{
	CLI::App* cli = SingletonCLI::GetInstance().GetCLI("Clustering.Kmeans");

	if (!configFilePath.empty())
	{
		SingletonCLI::GetInstance().AddConfigFile(configFilePath);
	}

	cli->add_option("--k", _cDefault.k, "K value for K-means clustering");
	cli->add_option("--kmeansEpsilon", _cDefault.kmeansEpsilon, "Difference criteria for K-means clustering");
	cli->add_option("--kmeansIteration", _cDefault.kmeansIteration, "Max iteration number for K-means clustering");
}

void Kmeans::TrainModel(
	const typename ClusteringBase<PointType>::DataListType& iTrainingData,
	typename ClusteringBase<PointType>::LabelsPtr oLabels,
	typename ClusteringBase<PointType>::ProbListPtr oProbabilities)
{	
	GlobalLogger logger;
	logger.Log({ {Tag::lvl, 3} }) << "Kmeans: Started. " << endl;

	// K-means clustering using dkm
	dkm::clustering_parameters<Real> dkm_params((unsigned int)_c.k);
	dkm_params.set_max_iteration(_c.kmeansIteration);
	dkm_params.set_min_delta(_c.kmeansEpsilon);

	vector<vector<Real>> means;
	auto [dkm_means, dkm_labels] = dkm::kmeans_lloyd_parallel(iTrainingData, dkm_params);
	//auto [dkm_means, dkm_labels] = dkm::kmeans_lloyd(samples, dkm_params);
	logger.Log({ {Tag::lvl, 3} }) << "Kmeans: K-means clustering finished." << endl;

	// save clustering results in oLabels
	oLabels->clear();
	for (auto& label : dkm_labels)
		oLabels->push_back(label);

	// compute likelihood: Not Implemented

	logger.Log({ {Tag::lvl, 3} }) << "Kmeans: Finished." << endl;

	// Training finished.
	ClusteringBase<PointType>::isTrained = true;
}

size_t Kmeans::GetClusterOf(
	const PointType& iTestValue,
	typename ClusteringBase<PointType>::ProbPtr oProbabilities)
{
	throw NotImplementedException;
}


