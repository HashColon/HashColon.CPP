#ifndef HASHCOLON_CLUSTERING_IMPL
#define HASHCOLON_CLUSTERING_IMPL

// HashColon config
#include <HashColon/HashColon_config.h>
// std libraries
#include <atomic>
#include <cassert>
#include <cmath>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <vector>
// dependant external libraries
#include <boost/type_index.hpp>
#include <Eigen/Eigen>
// modified external libraries
#include <CLI11_modified/CLI11_extended.hpp>
#include <dkm_modified/dkm_parallel.hpp>
// HashColon libraries
#include <HashColon/Exception.hpp>
#include <HashColon/Helper.hpp>
#include <HashColon/Log.hpp>
#include <HashColon/SingletonCLI.hpp>
// header file for this source file
#include <HashColon/Clustering.hpp>

// DistanceBasedClustering
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
				tempss << "Computing distances: " << progressCnt << "/" << a.size() << " " << Percentage(++progressCnt, (int)a.size());
				logger.Message << Flashl(tempss.str());
				//logger.Message << tempss.str() << endl;
			}
		}
		if (verbose)
			logger.Message << Flashl("") << flush;
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
		using namespace HashColon::LogUtils;		

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

// SpectralClustering
namespace HashColon::Clustering
{
	template<typename T>
	void SpectralClustering<T>::Initialize(const std::string configFilePath)
	{
		CLI::App* cli = HashColon::SingletonCLI::GetInstance().GetCLI("Clustering.SpectralClustering");

		if (!configFilePath.empty())
		{
			HashColon::SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--similaritySigma", _cDefault.similaritySigma, "Sigma value for converting distance to similarity");
		cli->add_option("--spaceSize", _cDefault.spaceSize, "Size of the spectral space. if given as 0, the value is chosen automatically. if given as negative value, maximum value(same as sample number) is chosen.");
	}

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
		using namespace HashColon;
		using namespace HashColon::LogUtils;

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
		CommonLogger logger;
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
				samples, oLabels, oProbabilities);

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

// NJW
namespace HashColon::Clustering
{
	template<typename T>
	void NJW<T>::Initialize(
		const std::string identifierPostfix,
		const std::string configFilePath)
	{
		using namespace std;
		using namespace HashColon::String;
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
		using namespace HashColon::String;

		CommonLogger logger;

		// if the given raw distance matrix is distance matrix, convert it to similarity matrix
		MatrixXR A = isDistance ? ConvertDistance2Similarity(iRawDistMatrix) : iRawDistMatrix;
		if (isDistance)
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

// DBSCAN
namespace HashColon::Clustering
{
	template<typename T>
	void DistanceBasedDBSCAN<T>::Initialize(
		const std::string identifierPostfix,
		const std::string configFilePath)
	{
		using namespace std;
		using namespace HashColon;
		using namespace HashColon::String;
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

		CLI::App* cli = SingletonCLI::GetInstance().GetCLI(identifier);

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
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
		Eigen::MatrixXR B = isDistance ? iRawDistMatrix : ConvertSimilarity2Distance(iRawDistMatrix);
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
			logger.Log({ { Tag::lvl, 3 } }) << this->GetMethodName() << ": Neighbor computation is finished." << endl;

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
		for (size_t i = 0; i < (size_t)iRawDistMatrix.cols(); i++)
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
					size_t clustersize = count_if(oLabels->begin(), oLabels->end(),
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
			logger.Log({ { Tag::lvl, 3} }) << this->GetMethodName() << ": Finished. (" << (_numOfClusters - 1) << " clusters + noise)" << endl;
			stringstream ss;
			for (size_t i = 0; i < _numOfClusters; i++)
			{
				size_t itemCnt = count_if(oLabels->begin(), oLabels->end(),
					[&i](auto e) { return e == i; });
				if (i == 0)
					ss << "Noise\t: " << itemCnt << " items\n";
				else
					ss << "C" << (i - 1) << "\t: " << itemCnt << " items\n";
			}
			logger.Log({ {Tag::lvl, 3} }) << "\n" << ss.str() << flush;
		}
	}

	template <typename T>
	HashColon::Real DistanceBasedDBSCAN<T>::ConvertSimilarity2Distance(const HashColon::Real& s) const
	{
		return s <= 0 ? std::numeric_limits<HashColon::Real>::max() : std::sqrt(-std::log(s));
	}

	template <typename T>
	Eigen::MatrixXR DistanceBasedDBSCAN<T>::ConvertSimilarity2Distance(const Eigen::MatrixXR& S) const
	{
		return S.unaryExpr(
			[this](HashColon::Real s) {
				return ConvertSimilarity2Distance(s);
			});
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
					if (labels[n] != clusterIdx)
						q.push(n);
				}
			}
		} while (!q.empty());
	}
}



#endif	
