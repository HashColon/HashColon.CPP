#ifndef HASHCOLON_CLUSTERING_NJW_HPP
#define HASHCOLON_CLUSTERING_NJW_HPP

#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif


#include <vector>
#include <Eigen/Eigen>

#include <HashColon/Clustering/ext/dkm/dkm_parallel.hpp>
#include <HashColon/Helper/Exception.hpp>
#include <HashColon/Helper/Real.hpp>
#include <HashColon/Helper/SingletonCLI.hpp>
#include <HashColon/Helper/CommonLogger.hpp>
#include <HashColon/Clustering/ClusteringBase.hpp>


namespace HashColon::Clustering
{
	template<typename DataType>
	class NJW : public DistanceBasedClustering<DataType>
	{
	public:
		struct _Params
		{
			HashColon::Helper::Real similaritySigma;
			size_t k;
			HashColon::Helper::Real kmeansEpsilon;
			size_t kmeansIteration;
		};

		HASHCOLON_CLASS_EXCEPTION_DEFINITION(NJW);

	protected:
		static _Params _cDefault;
		_Params _c;

	public:
		NJW(
			typename DistanceMeasureBase<DataType>::Ptr distanceFunction,
			_Params params = _cDefault)
			: DistanceBasedClustering<DataType>(distanceFunction), _c(params)
		{};

		static void Initialize(const std::string configFilePath = "")
		{			
			CLI::App* cli = HashColon::Helper::SingletonCLI::GetInstance().GetCLI("Clustering.NJW");

			if (!configFilePath.empty())
			{
				HashColon::Helper::SingletonCLI::GetInstance().AddConfigFile(configFilePath);
			}

			cli->add_option("--similaritySigma", _cDefault.similaritySigma, "Sigma value for converting distance to similarity");
			cli->add_option("--k", _cDefault.k, "K value for K-means clustering");
			cli->add_option("--kmeansEpsilon", _cDefault.kmeansEpsilon, "Difference criteria for K-means clustering");
			cli->add_option("--kmeansIteration", _cDefault.kmeansIteration, "Max iteration number for K-means clustering");
		}

		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };

	private:
		HashColon::Helper::Real ConvertDistance2Similarity(const HashColon::Helper::Real& d) const;
		Eigen::MatrixXR ConvertDistance2Similarity(const Eigen::MatrixXR D) const;
		Eigen::MatrixXR SpectralDomain;

	public:
		void TrainModel(
			const typename ClusteringBase<DataType>::DataListType& iTrainingData,
			typename ClusteringBase<DataType>::LabelsPtr oLabels = nullptr,
			typename ClusteringBase<DataType>::ProbListPtr oProbabilities = nullptr)
			override final;

		// get cluster label for a given data sample.
		// training most be done before using this function.
		size_t GetClusterOf(
			const DataType& iTestValue,
			typename ClusteringBase<DataType>::ProbPtr oProbabilities = nullptr)
			override final;

		// get number of cluster of the trained model.		
		int GetNumOfClusters() override final { return _c.k; };

		// erase trained model.
		void cleanup() override final { SpectralDomain.resize(0,0); ClusteringBase<DataType>::isTrained = false; };

		// get clustering method name in string
		const std::string GetMethodName() const final { return "NJW"; };
	};
}


// definitions
namespace HashColon::Clustering
{
	template<typename T>
	HashColon::Helper::Real NJW<T>::ConvertDistance2Similarity(const HashColon::Helper::Real& d) const
	{
		return std::exp((-.5) * d * d / _c.similaritySigma / _c.similaritySigma);
	}

	template<typename T>
	Eigen::MatrixXR NJW<T>::ConvertDistance2Similarity(const Eigen::MatrixXR D) const
	{
		return D.unaryExpr(
			[this](HashColon::Helper::Real d) {
				return ConvertDistance2Similarity(d);
			}) - Eigen::MatrixXR::Identity(D.rows(), D.cols());		
	}

	template<typename T>
	void NJW<T>::TrainModel(
		const typename ClusteringBase<T>::DataListType& iTrainingData,
		typename ClusteringBase<T>::LabelsPtr oLabels,
		typename ClusteringBase<T>::ProbListPtr oProbabilities)
	{
		using namespace std;
		using namespace Eigen;
		using namespace HashColon::Helper;

		using Tag = HashColon::Helper::LogUtils::Tag;
		using GlobalLogger = HashColon::Helper::CommonLogger;

		GlobalLogger logger;
		logger.Log({ {Tag::lvl, 1} }) << "NJW: Started. Using distance method: " << this->MeasureFunc->GetMethodName() << endl;		

		// compute similarity matrix A
		MatrixXR A = this->MeasureFunc->GetMeasureType() == DistanceMeasureType::similarity
			? DistanceBasedClustering<T>::ComputeDistanceMatrix(iTrainingData)
			: ConvertDistance2Similarity(DistanceBasedClustering<T>::ComputeDistanceMatrix(iTrainingData));
		logger.Log({ {Tag::lvl, 3} }) << "NJW: Similarity matrix computation finished." << endl;	

		logger.Debug({ {Tag::lvl, 1} }) << "\n" <<
			DistanceBasedClustering<T>::ComputeDistanceMatrix(iTrainingData) << endl;
		logger.Debug({ {Tag::lvl, 1} }) << "\n" << A << endl;

		// compute D^(-1/2) 
		MatrixXR D_half = A.rowwise().sum().unaryExpr(
			[](double a) {
				return 1 / sqrt(a);
			}).asDiagonal();

			logger.Debug({ {Tag::lvl, 1} }) << A.rowwise().sum() << endl;
			logger.Debug({ {Tag::lvl, 1} }) << D_half << endl;

		// compute normalized matrix L
		MatrixXR L = D_half * A * D_half;
		logger.Log({ {Tag::lvl, 3} }) << "NJW: Normalized matrix L computation finished." << endl;		

		logger.Debug({ {Tag::lvl, 1} }) << L << endl;

		// compute eigen vectors & values
		SelfAdjointEigenSolver<MatrixXR> eigenSolver(L);
		VectorXR lambda = eigenSolver.eigenvalues().tail(_c.k);
		SpectralDomain = eigenSolver.eigenvectors().rightCols(_c.k).colwise().normalized();

		logger.Debug({ {Tag::lvl, 1} }) << SpectralDomain << endl;

		// data conversion for kmeans clustering
		vector<vector<Real>> samples;
		samples.resize(SpectralDomain.rows());
		for (int i = 0; i < SpectralDomain.rows(); i++)
		{
			samples[i].resize(SpectralDomain.cols());
			for (int j = 0; j < SpectralDomain.cols(); j++)
				samples[i][j] = SpectralDomain(i, j);
		}
		logger.Log({ {Tag::lvl, 3} }) << "NJW: Eigen analysis of L finished." << endl;

		// K-means clustering using dkm
		dkm::clustering_parameters<Real> dkm_params(_c.k);		
		dkm_params.set_max_iteration(_c.kmeansIteration);
		dkm_params.set_min_delta(_c.kmeansEpsilon);

		vector<vector<Real>> means;		
		auto [dkm_means, dkm_labels] = dkm::kmeans_lloyd_parallel(samples, dkm_params);
		//auto [dkm_means, dkm_labels] = dkm::kmeans_lloyd(samples, dkm_params);
		logger.Log({ {Tag::lvl, 3} }) << "NJW: K-means clustering finished." << endl;		

		// save clustering results in oLabels
		oLabels->clear();
		for (auto& label : dkm_labels)
			oLabels->push_back(label);

		// compute likelihood: Not Implemented

		logger.Log({ {Tag::lvl, 3} }) << "NJW: Finished." << endl;		

		// Training finished.
		ClusteringBase<T>::isTrained = true;
	}

	template<typename T>
	size_t NJW<T>::GetClusterOf(
		const T& iTestValue,
		typename ClusteringBase<T>::ProbPtr oProbabilities)
	{
		throw NotImplementedException;
	}
}

// static member initialization
template <typename T>
typename HashColon::Clustering::NJW<T>::_Params HashColon::Clustering::NJW<T>::_cDefault;

#endif 
