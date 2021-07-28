#ifndef HASHCOLON_CLUSTERING_NJW_HPP
#define HASHCOLON_CLUSTERING_NJW_HPP

#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif


#include <vector>
#include <limits>
#include <Eigen/Eigen>

#include <HashColon/Core/Exception.hpp>
#include <HashColon/Core/Real.hpp>
#include <HashColon/Clustering/ClusteringBase.hpp>

namespace HashColon::Clustering
{
	template<typename DataType>
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
			: DistanceBasedClustering<DataType>(distanceFunction), _c(params)
		{};

		static void Initialize(
			const std::string identifierPostfix = "",
			const std::string configFilePath = "");
		
		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };

	private:
		HashColon::Real ConvertDistance2Similarity(const HashColon::Real& d) const;
		Eigen::MatrixXR ConvertDistance2Similarity(const Eigen::MatrixXR& D) const;
		Eigen::MatrixXR SpectralDomain;

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
			typename ClusteringBase<DataType>::ProbPtr oProbabilities = nullptr)
			override final;

		// get number of cluster of the trained model.		
		size_t GetNumOfClusters() override final { return _c.k; };

		// erase trained model.
		void cleanup() override final { SpectralDomain.resize(0,0); ClusteringBase<DataType>::isTrained = false; };

		// get clustering method name in string
		const std::string GetMethodName() const final { return "NJW"; };
	};
}

#endif 

#include <HashColon/Clustering/impl/NJW_Impl.hpp>