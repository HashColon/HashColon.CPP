#ifndef HASHCOLON_CLUSTERING
#define HASHCOLON_CLUSTERING

// std libraries
#include <array>
#include <limits>
#include <memory>
#include <utility>
#include <variant>
#include <vector>
// dependant external libraries
#include <Eigen/Eigen>
// HashColon libraries
#include <HashColon/Exception.hpp>
#include <HashColon/Real.hpp>
#include <HashColon/Statistics.hpp>

// ClusteringBase
namespace HashColon::Clustering
{
	template <typename DataType>
	class ClusteringBase
	{
		// class built-in types definition
	public:
		using Ptr = std::shared_ptr<ClusteringBase<DataType>>;
		using DataListType = std::vector<DataType>;
		using LabelsType = std::vector<size_t>;
		using LabelsPtr = std::shared_ptr<std::vector<size_t>>;
		using ProbType = std::vector<HashColon::Real>;
		using ProbPtr = std::shared_ptr<std::vector<HashColon::Real>>;
		using ProbListType = std::vector<std::vector<HashColon::Real>>;
		using ProbListPtr = std::shared_ptr<std::vector<std::vector<HashColon::Real>>>;

		// trains clustering model with given training data
		virtual void TrainModel(
			const DataListType &iTrainingData,
			LabelsPtr oLabels = nullptr,
			ProbListPtr oProbabilities = nullptr) = 0;

		// get cluster label for a given data sample.
		// training most be done before using this function.
		virtual size_t GetClusterOf(
			const DataType &iTestValue,
			ProbPtr oProbabilities = nullptr) = 0;

		// get number of cluster of the trained model.
		virtual size_t GetNumOfClusters() = 0;

		// erase trained model.
		virtual void cleanup() = 0;

		// get clustering method name in string
		virtual const std::string GetMethodName() const = 0;

		// check if this clustering model is done.
		// if TrainModel is done, this value is true. else, false.
		bool IsTrained() const { return isTrained; };

	protected:
		bool isTrained = false;
		ClusteringBase() : isTrained(false){};
	};
}

// DistanceBasedClustering
namespace HashColon::Clustering
{
	enum DistanceMeasureType
	{
		distance,
		similarity
	};

	template <typename DataType>
	class DistanceMeasureBase
	{
	public:
		using Ptr = std::shared_ptr<DistanceMeasureBase<DataType>>;
		virtual HashColon::Real Measure(const DataType &a, const DataType &b) const = 0;
		const DistanceMeasureType _measureType;
		const DistanceMeasureType GetMeasureType() const { return _measureType; };
		virtual const std::string GetMethodName() const = 0;

	protected:
		DistanceMeasureBase(DistanceMeasureType type) : _measureType(type){};
	};

	template <typename DataType>
	class DistanceBasedClustering : public ClusteringBase<DataType>
	{
	protected:
		typename DistanceMeasureBase<DataType>::Ptr MeasureFunc;

	public:
		HASHCOLON_CLASS_EXCEPTION_DEFINITION(DistanceBasedClustering);
		using Ptr = std::shared_ptr<DistanceBasedClustering<DataType>>;
		typename DistanceMeasureBase<DataType>::Ptr GetDistanceFunc() { return MeasureFunc; };
		typename DistanceMeasureBase<DataType>::Ptr GetSimilarityFunc() { return MeasureFunc; };

		virtual void TrainModel(
			const typename ClusteringBase<DataType>::DataListType &iTrainingData,
			typename ClusteringBase<DataType>::LabelsPtr oLabels = nullptr,
			typename ClusteringBase<DataType>::ProbListPtr oProbabilities = nullptr) override;

		virtual void TrainModel(
			const Eigen::MatrixXR &iRawDistanceMatrix, bool isDistance,
			typename ClusteringBase<DataType>::LabelsPtr oLabels = nullptr,
			typename ClusteringBase<DataType>::ProbListPtr oProbabilities = nullptr) = 0;

		Eigen::MatrixXR ComputeDistanceMatrix(
			const typename ClusteringBase<DataType>::DataListType &iTrainingData, bool verbose = false) const;

	protected:
		DistanceBasedClustering(typename DistanceMeasureBase<DataType>::Ptr func)
			: MeasureFunc(func){};
	};

	template <typename DataType>
	using SimilarityBasedClustering = DistanceBasedClustering<DataType>;

}

// PointBasedClustering
namespace HashColon::Clustering
{
	class PointBasedClustering : public ClusteringBase<std::vector<HashColon::Real>>
	{
	};
}

// NJW
namespace HashColon::Clustering
{
	template <typename DataType>
	class NJW : public DistanceBasedClustering<DataType>
	{
	public:
		struct _Params
		{
			HashColon::Real similaritySigma;
			size_t k;
			HashColon::Real kmeansEpsilon;
			size_t kmeansIteration;
		};

		HASHCOLON_CLASS_EXCEPTION_DEFINITION(NJW);

	protected:
		static inline _Params _cDefault;
		_Params _c;

	public:
		NJW(
			typename DistanceMeasureBase<DataType>::Ptr distanceFunction,
			_Params params = _cDefault)
			: DistanceBasedClustering<DataType>(distanceFunction), _c(params){};

		static void Initialize(
			const std::string identifierPostfix = "",
			const std::string configFilePath = "");

		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };

	private:
		HashColon::Real ConvertDistance2Similarity(const HashColon::Real &d) const;
		Eigen::MatrixXR ConvertDistance2Similarity(const Eigen::MatrixXR &D) const;
		Eigen::MatrixXR SpectralDomain;

	public:
		void TrainModel(
			const Eigen::MatrixXR &iRawDistanceMatrix, bool isDistance,
			typename ClusteringBase<DataType>::LabelsPtr oLabels = nullptr,
			typename ClusteringBase<DataType>::ProbListPtr oProbabilities = nullptr)
			override final;

		// get cluster label for a given data sample.
		// training most be done before using this function.
		size_t GetClusterOf(
			const DataType &iTestValue,
			typename ClusteringBase<DataType>::ProbPtr oProbabilities = nullptr)
			override final;

		// get number of cluster of the trained model.
		size_t GetNumOfClusters() override final { return _c.k; };

		// erase trained model.
		void cleanup() override final
		{
			SpectralDomain.resize(0, 0);
			ClusteringBase<DataType>::isTrained = false;
		};

		// get clustering method name in string
		const std::string GetMethodName() const final { return "NJW"; };
	};
}

// Kmeans
namespace HashColon::Clustering
{
	class Kmeans : public ClusteringBase<std::vector<HashColon::Real>>
	{
	public:
		struct _Params
		{
			size_t k;
			HashColon::Real kmeansEpsilon;
			size_t kmeansIteration;
		};

		HASHCOLON_CLASS_EXCEPTION_DEFINITION(Kmeans);

	protected:
		static inline _Params _cDefault;
		_Params _c;

	public:
		Kmeans(_Params params = _cDefault) : _c(params){};

		static void Initialize(const std::string configFilePath = "");

		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };

	public:
		void TrainModel(
			const typename ClusteringBase<std::vector<HashColon::Real>>::DataListType &iTrainingData,
			typename ClusteringBase<std::vector<HashColon::Real>>::LabelsPtr oLabels = nullptr,
			typename ClusteringBase<std::vector<HashColon::Real>>::ProbListPtr oProbabilities = nullptr)
			override final;

		// get cluster label for a given data sample.
		// training most be done before using this function.
		size_t GetClusterOf(
			const std::vector<HashColon::Real> &iTestValue,
			typename ClusteringBase<std::vector<HashColon::Real>>::ProbPtr oProbabilities = nullptr)
			override final;

		// get number of cluster of the trained model.
		size_t GetNumOfClusters() override final { return _c.k; };

		// erase trained model.
		void cleanup() override final { ClusteringBase<std::vector<HashColon::Real>>::isTrained = false; };

		// get clustering method name in string
		const std::string GetMethodName() const final { return "Kmeans"; };
	};
}

// DBSCAN
namespace HashColon::Clustering
{
	class DBSCAN : public PointBasedClustering
	{
	public:
		struct _Params
		{
			size_t minPts;
			HashColon::Real DbscanEpsilon;
		};

		HASHCOLON_CLASS_EXCEPTION_DEFINITION(DBSCAN);

	protected:
		static inline _Params _cDefault;
		_Params _c;
		size_t _numOfClusters;

	public:
		DBSCAN(_Params params = _cDefault) : _c(params){};

		static void Initialize(const std::string configFilePath = "");

		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };

	public:
		void TrainModel(
			const typename ClusteringBase<std::vector<HashColon::Real>>::DataListType &iTrainingData,
			typename ClusteringBase<std::vector<HashColon::Real>>::LabelsPtr oLabels = nullptr,
			typename ClusteringBase<std::vector<HashColon::Real>>::ProbListPtr oProbabilities = nullptr)
			override final;

		// get cluster label for a given data sample.
		// training most be done before using this function.
		size_t GetClusterOf(
			const std::vector<HashColon::Real> &iTestValue,
			typename ClusteringBase<std::vector<HashColon::Real>>::ProbPtr oProbabilities = nullptr)
			override final
		{
			throw NotImplementedException;
		}

		// get number of cluster of the trained model.
		size_t GetNumOfClusters() override final
		{
			return _numOfClusters;
		}

		// erase trained model.
		void cleanup() override final
		{
			throw NotImplementedException;
		}

		// get clustering method name in string
		const std::string GetMethodName() const final { return "DBSCAN"; };

	private:
		constexpr static size_t unclassified = 0;
		constexpr static size_t noise = 1;

		std::vector<std::vector<size_t>> GetNeighbors(
			const typename ClusteringBase<std::vector<HashColon::Real>>::DataListType &iTrainingData) const;
		void DbscanBfs(size_t initP, size_t clusterIdx,
					   std::vector<std::vector<size_t>> &neighbors, std::vector<size_t> &labels) const;
	};
}

// DistanceBasedDBSCAN
namespace HashColon::Clustering
{
	template <typename DataType>
	class DistanceBasedDBSCAN : public DistanceBasedClustering<DataType>
	{
	public:
		struct _Params
		{
			size_t minPts;
			HashColon::Real DbscanEpsilon;
			bool Verbose;
		};

		HASHCOLON_CLASS_EXCEPTION_DEFINITION(DistanceBasedDBSCAN);

	protected:
		static inline _Params _cDefault;
		_Params _c;
		size_t _numOfClusters;

	public:
		DistanceBasedDBSCAN(
			typename DistanceMeasureBase<DataType>::Ptr distanceFunction,
			_Params params = _cDefault)
			: DistanceBasedClustering<DataType>(distanceFunction), _c(params){};

		static void Initialize(
			const std::string identifierPostfix = "",
			const std::string configFilePath = "");

		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };

	public:
		void TrainModel(
			const Eigen::MatrixXR &iRawDistanceMatrix, bool isDistance,
			typename ClusteringBase<DataType>::LabelsPtr oLabels = nullptr,
			typename ClusteringBase<DataType>::ProbListPtr oProbabilities = nullptr)
			override final;

		// get cluster label for a given data sample.
		// training most be done before using this function.
		size_t GetClusterOf(
			const DataType &iTestValue,
			typename ClusteringBase<std::vector<HashColon::Real>>::ProbPtr oProbabilities = nullptr)
			override final
		{
			throw NotImplementedException;
		}

		// get number of cluster of the trained model.
		size_t GetNumOfClusters() override final
		{
			return _numOfClusters;
		}

		// erase trained model.
		void cleanup() override final
		{
			_numOfClusters = 0;
		}

		// get clustering method name in string
		const std::string GetMethodName() const final { return "DistanceBasedDBSCAN"; };

	private:
		constexpr static size_t unclassified = 0;
		constexpr static size_t noise = 1;

		HashColon::Real ConvertSimilarity2Distance(const HashColon::Real &s) const;
		Eigen::MatrixXR ConvertSimilarity2Distance(const Eigen::MatrixXR &S) const;

		std::vector<std::vector<size_t>> GetNeighbors(const Eigen::MatrixXR &DistMatrix) const;
		void DbscanBfs(size_t initP, size_t clusterIdx,
					   std::vector<std::vector<size_t>> &neighbors, std::vector<size_t> &labels) const;
	};
}

// SpectralClustering
namespace HashColon::Clustering
{
	template <typename DataType>
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
		inline static _Params _cDefault;
		_Params _c;
		typename ClusteringBase<std::vector<HashColon::Real>>::Ptr _internalClustering;

	public:
		SpectralClustering(
			typename DistanceMeasureBase<DataType>::Ptr distanceFunction,
			typename ClusteringBase<std::vector<HashColon::Real>>::Ptr internalClusteringMethod,
			_Params params = _cDefault)
			: DistanceBasedClustering<DataType>(distanceFunction),
			  _internalClustering(internalClusteringMethod),
			  _c(params){};

		static void Initialize(const std::string configFilePath = "");

		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };

	private:
		HashColon::Real ConvertDistance2Similarity(const HashColon::Real &d) const;
		Eigen::MatrixXR ConvertDistance2Similarity(const Eigen::MatrixXR D) const;
		Eigen::MatrixXR SpectralDomain;

	public:
		void TrainModel(
			const typename ClusteringBase<DataType>::DataListType &iTrainingData,
			typename ClusteringBase<DataType>::LabelsPtr oLabels = nullptr,
			typename ClusteringBase<DataType>::ProbListPtr oProbabilities = nullptr)
			override final;

		// get cluster label for a given data sample.
		// training most be done before using this function.
		size_t GetClusterOf(
			const DataType &iTestValue,
			typename ClusteringBase<DataType>::ProbPtr oProbabilities = nullptr)
			override final;

		// get number of cluster of the trained model.
		int GetNumOfClusters() override final { return _c.spaceSize; };

		// erase trained model.
		void cleanup() override final
		{
			SpectralDomain.resize(0, 0);
			ClusteringBase<DataType>::isTrained = false;
		};

		// get clustering method name in string
		const std::string GetMethodName() const final { return "SpectralClustering"; };
	};
}

// Evaluation functions for clustering
namespace HashColon::Clustering
{
	struct DistancesAnalysisResults
	{
		std::vector<HashColon::Statistics::SimpleStatisticsAnalysisResults> ClusterResults;
		HashColon::Statistics::SimpleStatisticsAnalysisResults TotalResults;
	};

	HashColon::Statistics::SimpleStatisticsAnalysisResults DistanceAnalysis(const std::vector<HashColon::Real> &DistancesInCluster);

	DistancesAnalysisResults DistanceAnalysis(
		const std::vector<size_t> &clusterResult, const Eigen::MatrixXR &DistanceMatrix);

	std::vector<std::vector<HashColon::Real>> SortedDistanceGraph(
		const std::vector<size_t> &clusterResult, const Eigen::MatrixXR &DistanceMatrix);

	std::vector<std::vector<HashColon::Real>> SortedDistanceGraph(
		const std::vector<size_t> &clusterResult, const Eigen::MatrixXR &DistanceMatrix,
		DistancesAnalysisResults &additionals);

	std::vector<size_t> PseudoMedian(
		const std::vector<size_t> &clusterResult, const Eigen::MatrixXR &DistanceMatrix);

	/*
	 * The Original definition of Davies-Bouldin index of a cluster is vary similar to standard deviation of a clsuter
	 *
	 * DBI_i = sqrt ( 1/N_i ( sum( ( X_k - C_i ) ^ 2 ) )
	 */
	std::vector<HashColon::Real> PseudoDaviesBouldin(
		const std::vector<size_t> &clusterResult, const Eigen::MatrixXR &DistanceMatrix);

	std::vector<HashColon::Real> PseudoDaviesBouldin(
		const std::vector<size_t> &clusterResult, const Eigen::MatrixXR &DistanceMatrix,
		std::vector<size_t> &pseudoMedian);

	// silhouette for single data
	HashColon::Real Silhouette(size_t itemIdx, const std::vector<size_t> &clusterResult, const Eigen::MatrixXR &DistanceMatrix);

	// silhouette for all data
	std::vector<HashColon::Real> Silhouette(const std::vector<size_t> &clusterResult, const Eigen::MatrixXR &DistanceMatrix);
	// HashColon::Real ClassificationError(const std::vector<size_t>& clusterResult, const std::vector<size_t>& givenLabel);
	// HashColon::Real VariationOfInfomration(const std::vector<size_t>& clusterResult, const std::vector<size_t>& givenLabel);
}

#endif

#include <HashColon/impl/Clustering_Impl.hpp>
