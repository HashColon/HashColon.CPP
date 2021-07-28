#ifndef HASHCOLON_CLUSTERING_NJW_IMPL
#define HASHCOLON_CLUSTERING_NJW_IMPL

#include <HashColon/Clustering/NJW.hpp>

#include <boost/type_index.hpp>
#include <dkm_modified/dkm_parallel.hpp>
#include <HashColon/Core/SingletonCLI.hpp>
#include <HashColon/Helper/StringHelper.hpp>
#include <HashColon/Core/CommonLogger.hpp>


namespace HashColon::Clustering
{
	template<typename T>
	void NJW<T>::Initialize(
		const std::string identifierPostfix,
		const std::string configFilePath)
	{
		using namespace std;
		using namespace HashColon::Helper;
		using namespace boost::typeindex;

		string identifier = "Clustering.NJW";
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
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--similaritySigma", _cDefault.similaritySigma, "Sigma value for converting distance to similarity");
		cli->add_option("--k", _cDefault.k, "K value for K-means clustering");
		cli->add_option("--kmeansEpsilon", _cDefault.kmeansEpsilon, "Difference criteria for K-means clustering");
		cli->add_option("--kmeansIteration", _cDefault.kmeansIteration, "Max iteration number for K-means clustering");
	}

	template<typename T>
	HashColon::Real NJW<T>::ConvertDistance2Similarity(const HashColon::Real& d) const
	{
		return std::exp((-.5) * d * d / _c.similaritySigma / _c.similaritySigma);
	}

	template<typename T>
	Eigen::MatrixXR NJW<T>::ConvertDistance2Similarity(const Eigen::MatrixXR& D) const
	{
		return D.unaryExpr(
			[this](HashColon::Real d) {
				return ConvertDistance2Similarity(d);
			}) - Eigen::MatrixXR::Identity(D.rows(), D.cols());
	}

	template<typename T>
	void NJW<T>::TrainModel(
		const Eigen::MatrixXR& iRawDistMatrix, bool isDistance,
		typename ClusteringBase<T>::LabelsPtr oLabels,
		typename ClusteringBase<T>::ProbListPtr oProbabilities)
	{
		using namespace std;
		using namespace Eigen;
		using namespace HashColon;
		using namespace HashColon::LogUtils;
		using namespace HashColon::Helper;
				
		CommonLogger logger;
		
		// if the given raw distance matrix is distance matrix, convert it to similarity matrix
		MatrixXR A = isDistance ? ConvertDistance2Similarity(iRawDistMatrix) : iRawDistMatrix;
		if(isDistance)
		{
			lock_guard<mutex> _lg(CommonLogger::_mutex);
			logger.Log({ { Tag::lvl, 3 } }) << this->GetMethodName() << ": Raw distance matrix conversion from distance to similarity is finished. " << endl;
			logger.Debug({ {__CODEINFO_TAGS__} }) << "\n" <<
				"Converted simlarity matrix A:\n" << A << endl;
		}

		// compute D^(-1/2) 
		MatrixXR D_half = A.rowwise().sum().unaryExpr(
			[](double a) {
				return (a == 0) ? (std::numeric_limits<Real>::max()) : (1 / sqrt(a));
			}
		).asDiagonal();
		{
			lock_guard<mutex> _lg(CommonLogger::_mutex);
			logger.Debug({ __CODEINFO_TAGS__ }) << "\nRowwise sum of A:\n" << A.rowwise().sum() << endl;
			logger.Debug({ __CODEINFO_TAGS__ }) << "\nD_half:\n" << D_half << endl;
		}

		// compute normalized matrix L
		MatrixXR L = D_half * A * D_half;
		{
			lock_guard<mutex> _lg(CommonLogger::_mutex);
			logger.Log({ {Tag::lvl, 3} }) << this->GetMethodName() << ": Normalized matrix L computation finished." << endl;
			logger.Debug({ __CODEINFO_TAGS__ }) << "\nL:\n" << L << endl;
		}

		// compute eigen vectors & values
		SelfAdjointEigenSolver<MatrixXR> eigenSolver(L);
		VectorXR lambda = eigenSolver.eigenvalues().tail(_c.k);
		SpectralDomain = eigenSolver.eigenvectors().rightCols(_c.k).colwise().normalized();

		{
			lock_guard<mutex> _lg(CommonLogger::_mutex);
			logger.Debug({ __CODEINFO_TAGS__ }) << "\nSpectral domain:\n" << SpectralDomain << endl;
		}

		// data conversion for kmeans clustering
		vector<vector<Real>> samples;
		samples.resize(SpectralDomain.rows());
		for (int i = 0; i < SpectralDomain.rows(); i++)
		{
			samples[i].resize(SpectralDomain.cols());
			for (int j = 0; j < SpectralDomain.cols(); j++)
				samples[i][j] = SpectralDomain(i, j);
		}
		{
			lock_guard<mutex> _lg(CommonLogger::_mutex);
			logger.Log({ {Tag::lvl, 3} }) << this->GetMethodName() << ": Eigen analysis of L finished." << endl;
		}

		// K-means clustering using dkm
		dkm::clustering_parameters<Real> dkm_params((unsigned int)_c.k);
		dkm_params.set_max_iteration(_c.kmeansIteration);
		dkm_params.set_min_delta(_c.kmeansEpsilon);

		vector<vector<Real>> means;
		auto [dkm_means, dkm_labels] = dkm::kmeans_lloyd_parallel(samples, dkm_params);
		//auto [dkm_means, dkm_labels] = dkm::kmeans_lloyd(samples, dkm_params);
		{
			lock_guard<mutex> _lg(CommonLogger::_mutex);
			logger.Log({ {Tag::lvl, 3} }) << this->GetMethodName() << ": K-means clustering finished." << endl;
		}

		// save clustering results in oLabels
		oLabels->clear();
		for (auto& label : dkm_labels)
			oLabels->push_back(label);

		// compute likelihood: Not Implemented
		{
			lock_guard<mutex> _lg(CommonLogger::_mutex);
			logger.Log({ {Tag::lvl, 3} }) << this->GetMethodName() << ": Finished." << endl;
		}

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

#endif
