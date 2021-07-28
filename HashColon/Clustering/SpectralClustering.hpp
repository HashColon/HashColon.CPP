#ifndef HASHCOLON_CLUSTERING_SPECTRALCLUSTERING_HPP
#define HASHCOLON_CLUSTERING_SPECTRALCLUSTERING_HPP

#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif


#include <vector>
#include <Eigen/Eigen>

#include <dkm_modified/dkm_parallel.hpp>
#include <HashColon/Core/Exception.hpp>
#include <HashColon/Core/Real.hpp>
#include <HashColon/Core/SingletonCLI.hpp>
#include <HashColon/Core/CommonLogger.hpp>
#include <HashColon/Clustering/ClusteringBase.hpp>

//TODO: Declaration / Definition separation needed

namespace HashColon::Clustering
{
	template<typename DataType>
	class SpectralClustering : public DistanceBasedClustering<DataType>
	{
	public:
		struct _Params
		{
			HashColon::Real similaritySigma;		
			HashColon::Real spaceSize;
		};

		HASHCOLON_CLASS_EXCEPTION_DEFINITION(SpectralClustering);

	protected:
		static _Params _cDefault;
		_Params _c;
		typename ClusteringBase<std::vector<HashColon::Real>::Ptr _internalClustering;

	public:
		SpectralClustering(
			typename DistanceMeasureBase<DataType>::Ptr distanceFunction,
			typename ClusteringBase<std::vector<HashColon::Real>::Ptr internalClusteringMethod,
			_Params params = _cDefault)
			: DistanceBasedClustering<DataType>(distanceFunction), 
			_internalClustering(internalClusteringMethod),
			_c(params)
		{};

		static void Initialize(const std::string configFilePath = "")
		{
			CLI::App* cli = HashColon::SingletonCLI::GetInstance().GetCLI("Clustering.SpectralClustering");

			if (!configFilePath.empty())
			{
				HashColon::SingletonCLI::GetInstance().AddConfigFile(configFilePath);
			}

			cli->add_option("--similaritySigma", _cDefault.similaritySigma, "Sigma value for converting distance to similarity");			
			cli->add_option("--spaceSize", _cDefault.spaceSize, "Size of the spectral space. if given as 0, the value is chosen automatically. if given as negative value, maximum value(same as sample number) is chosen.");
		}

		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };

	private:
		HashColon::Real ConvertDistance2Similarity(const HashColon::Real& d) const;
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
		int GetNumOfClusters() override final { return _c.spaceSize; };

		// erase trained model.
		void cleanup() override final { SpectralDomain.resize(0, 0); ClusteringBase<DataType>::isTrained = false; };

		// get clustering method name in string
		const std::string GetMethodName() const final { return "SpectralClustering"; };
	};
}


// definitions
namespace HashColon::Clustering
{
	template<typename T>
	HashColon::Real SpectralClustering<T>::ConvertDistance2Similarity(const HashColon::Real& d) const
	{
		return std::exp((-.5) * d * d / _c.similaritySigma / _c.similaritySigma);
	}

	template<typename T>
	Eigen::MatrixXR SpectralClustering<T>::ConvertDistance2Similarity(const Eigen::MatrixXR D) const
	{
		return D.unaryExpr(
			[this](HashColon::Real d) {
				return ConvertDistance2Similarity(d);
			}) - Eigen::MatrixXR::Identity(D.rows(), D.cols());
	}

	template<typename T>
	void SpectralClustering<T>::TrainModel(
		const typename ClusteringBase<T>::DataListType& iTrainingData,
		typename ClusteringBase<T>::LabelsPtr oLabels,
		typename ClusteringBase<T>::ProbListPtr oProbabilities)
	{
		using namespace std;
		using namespace Eigen;
		using namespace HashColon::Helper;

		using Tag = HashColon::LogUtils::Tag;
		using GlobalLogger = HashColon::CommonLogger;

		// !!Assertions
		// check if training is not done yet 
		assert(!ClusteringBase<T>::isTrained);;
		// check if the measure function / internal clustering methods are given
		assert(this->MeasureFunc != nullptr);
		assert(this->_internalClustering != nullptr);		
		// check if the data is given
		assert(iTrainingData.size() > 0);
		// if measure is given as similarity, than check if the sigma is not zero
		assert(!(this->MeasureFunc->GetMeasureType() == DistanceMeasureType::similarity && _c.similaritySigma == 0));

		// Get this shit runnin'
		GlobalLogger logger;
		logger.Log({ {Tag::lvl, 1} }) 
			<< "SpectralClustering: Started. Using distance method: " << this->MeasureFunc->GetMethodName() 
			<< " / Using internal clustering method: " << this->_internalClustering->GetMethodName()
			<< endl;

		// compute similarity matrix A
		MatrixXR A = this->MeasureFunc->GetMeasureType() == DistanceMeasureType::similarity
			? DistanceBasedClustering<T>::ComputeDistanceMatrix(iTrainingData)
			: ConvertDistance2Similarity(DistanceBasedClustering<T>::ComputeDistanceMatrix(iTrainingData));
		logger.Log({ {Tag::lvl, 3} }) << "SpectralClustering: Similarity matrix computation finished." << endl;

		logger.Debug({ __CODEINFO_TAGS__ }) << "\n" <<
			"Raw distance matrix: " << "\n" <<
			DistanceBasedClustering<T>::ComputeDistanceMatrix(iTrainingData) << endl;
		logger.Debug({ __CODEINFO_TAGS__ }) << "\n" <<
			"A:\n" << A << endl;

		// compute D^(-1/2) 
		MatrixXR D_half = A.rowwise().sum().unaryExpr(
			[](double a) {
				return 1 / sqrt(a);
			}).asDiagonal();

		logger.Debug({ __CODEINFO_TAGS__ }) << "\nRowwise sum of A:\n" << A.rowwise().sum() << endl;
		logger.Debug({ __CODEINFO_TAGS__ }) << "\nD_half:\n" << D_half << endl;

		// compute normalized matrix L
		MatrixXR L = D_half * A * D_half;
		logger.Log({ {Tag::lvl, 3} }) << "SpectralClustering: Normalized matrix L computation finished." << endl;

		logger.Debug({ __CODEINFO_TAGS__ }) << "\nL:\n" << L << endl;

		// compute eigen vectors & values
		SelfAdjointEigenSolver<MatrixXR> eigenSolver(L);

		size_t N = iTrainingData.size();
		VectorXR lambda = eigenSolver.eigenvalues();		
		// cut down spectral space 
		// if k is not in [0, N] then k = N
		if (_c.spaceSize < 0 || _c.spaceSize > N) {
			_c.spaceSize = N;
		}
		// auto space cut
		else if (_c.spaceSize == 0)
		{
			size_t maxDiffIdx = N - 1;
			Real maxDiff = 0;
			for (size_t i = N - 1; i > 0; i--)
			{
				if (maxDiff < (lambda[i] - lambda[i - 1]))
				{
					maxDiff = (lambda[i] - lambda[i - 1]);
					maxDiffIdx = i;
				}
			}
			_c.spaceSize = N - maxDiffIdx;
		}
		
		// cut down spectral space and normalize
		lambda = lambda.tail(_c.spaceSize);
		SpectralDomain = SpectralDomain = eigenSolver.eigenvectors().rightCols(_c.spaceSize).colwise().normalized();

		logger.Debug({ __CODEINFO_TAGS__ }) << "\nSpectral domain:\n" << SpectralDomain << endl;
		// data conversion for kmeans clustering
		vector<vector<Real>> samples;
		samples.resize(SpectralDomain.rows());
		for (int i = 0; i < SpectralDomain.rows(); i++)
		{
			samples[i].resize(SpectralDomain.cols());
			for (int j = 0; j < SpectralDomain.cols(); j++)
				samples[i][j] = SpectralDomain(i, j);
		}
		logger.Log({ {Tag::lvl, 3} }) << "SpectralClustering: Eigen analysis of L finished." << endl;

		// run internal clustering
		this->_internalClustering()->TrainModel(
			samples, oLabels, oProbabilites);
		
		logger.Log({ {Tag::lvl, 3} }) << "SpectralClustering: internal clustering method(" <<
			this->_internalClustering->GetMethodName()
			<< ") finished." << endl;
		
		logger.Log({ {Tag::lvl, 3} }) << "SpectralClustering: Finished." << endl;
		// Training finished.
		ClusteringBase<T>::isTrained = true;
	}

	template<typename T>
	size_t SpectralClustering<T>::GetClusterOf(
		const T& iTestValue,
		typename ClusteringBase<T>::ProbPtr oProbabilities)
	{
		throw NotImplementedException;
	}
}

// static member initialization
template <typename T>
typename HashColon::Clustering::SpectralClustering<T>::_Params HashColon::Clustering::SpectralClustering<T>::_cDefault;

#endif 

