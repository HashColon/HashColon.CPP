#ifndef HASHCOLON_CLUSTERING_CLUSTERINGBASE_HPP
#define HASHCOLON_CLUSTERING_CLUSTERINGBASE_HPP

#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif

#include <array>
#include <vector>
#include <memory>
#include <variant>
#include <utility>
#include <Eigen/Eigen>
#include <HashColon/Core/Real.hpp>
#include <HashColon/Core/Exception.hpp>

namespace HashColon::Clustering
{
	template<typename DataType>
	class ClusteringBase
	{
		// class built-in types definition
	public:
		using Ptr = std::shared_ptr<ClusteringBase<DataType>>;
		using DataListType = std::vector<DataType>;
		using LabelsType = std::vector<size_t>;
		using LabelsPtr = std::shared_ptr<std::vector<size_t>>;
		using ProbType = std::vector<HashColon::Real>;
		using ProbPtr = std::shared_ptr< std::vector<HashColon::Real>>;
		using ProbListType = std::vector<std::vector<HashColon::Real>>;
		using ProbListPtr = std::shared_ptr<std::vector<std::vector<HashColon::Real>>>;

		// trains clustering model with given training data		
		virtual void TrainModel(
			const DataListType& iTrainingData,
			LabelsPtr oLabels = nullptr,
			ProbListPtr oProbabilities = nullptr)
			= 0;

		// get cluster label for a given data sample.
		// training most be done before using this function.
		virtual size_t GetClusterOf(
			const DataType& iTestValue,
			ProbPtr oProbabilities = nullptr)
			= 0;

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
		ClusteringBase() : isTrained(false) {};
	};
}

namespace HashColon::Clustering
{
	enum DistanceMeasureType { distance, similarity };

	template<typename DataType>
	class DistanceMeasureBase
	{
	public:
		using Ptr = std::shared_ptr<DistanceMeasureBase<DataType>>;
		virtual HashColon::Real Measure(const DataType& a, const DataType& b) const = 0;
		const DistanceMeasureType _measureType;
		const DistanceMeasureType GetMeasureType() const { return _measureType; };
		virtual const std::string GetMethodName() const = 0;
	protected:
		DistanceMeasureBase(DistanceMeasureType type) : _measureType(type) {};
	};


	template<typename DataType>
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
			const typename ClusteringBase<DataType>::DataListType& iTrainingData,
			typename ClusteringBase<DataType>::LabelsPtr oLabels = nullptr,
			typename ClusteringBase<DataType>::ProbListPtr oProbabilities = nullptr) override;

		virtual void TrainModel(
			const Eigen::MatrixXR& iRawDistanceMatrix,	bool isDistance,
			typename ClusteringBase<DataType>::LabelsPtr oLabels = nullptr,
			typename ClusteringBase<DataType>::ProbListPtr oProbabilities = nullptr) = 0;

		Eigen::MatrixXR ComputeDistanceMatrix(
			const typename ClusteringBase<DataType>::DataListType& iTrainingData, bool verbose = false) const;
			
	protected:
		DistanceBasedClustering(typename DistanceMeasureBase<DataType>::Ptr func)
			: MeasureFunc(func) {};
	};

	template <typename DataType>
	using SimilarityBasedClustering = DistanceBasedClustering<DataType>;

	

}

namespace HashColon::Clustering
{	
	class PointBasedClustering : public ClusteringBase<std::vector<HashColon::Real>>
	{	
	};
}

#endif

#include <HashColon/Clustering/impl/ClusteringBase_Impl.hpp>
