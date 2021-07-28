#ifndef HASHCOLON_CLUSTERING_KMEANS_HPP
#define HASHCOLON_CLUSTERING_KMEANS_HPP

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
		Kmeans(_Params params = _cDefault) : _c(params) {};

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
			override final;

		// get number of cluster of the trained model.		
		size_t GetNumOfClusters() override final { return _c.k; };

		// erase trained model.
		void cleanup() override final { ClusteringBase<std::vector<HashColon::Real>>::isTrained = false; };

		// get clustering method name in string
		const std::string GetMethodName() const final { return "Kmeans"; };
	};
}

#endif 
