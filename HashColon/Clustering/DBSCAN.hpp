#ifndef HASHCOLON_CLUSTERING_DBSCAN_HPP
#define HASHCOLON_CLUSTERING_DBSCAN_HPP

#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif

#include <vector>
#include <Eigen/Eigen>

#include <HashColon/Core/Exception.hpp>
#include <HashColon/Core/Real.hpp>
#include <HashColon/Clustering/ClusteringBase.hpp>

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
		DBSCAN(_Params params = _cDefault) : _c(params) {};

		static void Initialize(const std::string configFilePath = "");

		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };

	public:
		void TrainModel(
			const typename ClusteringBase<std::vector<HashColon::Real>>::DataListType& iTrainingData,
			typename ClusteringBase<std::vector<HashColon::Real>>::LabelsPtr oLabels = nullptr,
			typename ClusteringBase<std::vector<HashColon::Real>>::ProbListPtr oProbabilities = nullptr)
			override final;

		// get cluster label for a given data sample.
		// training most be done before using this function.
		size_t GetClusterOf(
			const std::vector<HashColon::Real>& iTestValue,
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
			const typename ClusteringBase<std::vector<HashColon::Real>>::DataListType& iTrainingData ) const;
		void DbscanBfs(size_t initP, size_t clusterIdx, 
			std::vector<std::vector<size_t>>& neighbors, std::vector<size_t>& labels) const;

	};
}

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
			: DistanceBasedClustering<DataType>(distanceFunction), _c(params)
		{};

		static void Initialize(
			const std::string identifierPostfix = "",
			const std::string configFilePath = "");

		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };

	public:
		void TrainModel(
			const Eigen::MatrixXR& iRawDistanceMatrix, bool isDistance,
			typename ClusteringBase<DataType>::LabelsPtr oLabels = nullptr,
			typename ClusteringBase<DataType>::ProbListPtr oProbabilities = nullptr)
			override final;

		// get cluster label for a given data sample.
		// training most be done before using this function.
		size_t GetClusterOf(
			const DataType& iTestValue,
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
		const std::string GetMethodName() const final { return "DistanceBasedDBSCAN"; };

	private:
		constexpr static size_t unclassified = 0;
		constexpr static size_t noise = 1;

		Eigen::MatrixXR ConvertSimilarity2Distance(const Eigen::MatrixXR& D) const
		{
			throw NotImplementedException;
		}

		std::vector<std::vector<size_t>> GetNeighbors(const Eigen::MatrixXR& DistMatrix) const;
		void DbscanBfs(size_t initP, size_t clusterIdx, 
			std::vector<std::vector<size_t>>& neighbors, std::vector<size_t>& labels) const;
			
	};
}

#endif	

#include <HashColon/Clustering/impl/DBSCAN_Impl.hpp>