#ifndef HASHCOLON_FELINE_TRAJECTORYCLUSTERING_HPP
#define HASHCOLON_FELINE_TRAJECTORYCLUSTERING_HPP

// std libraries
#include <memory>
#include <vector>
// dependant external libraries
#include <Eigen/Eigen>
// HashColon libraries
#include <HashColon/Clustering.hpp>
#include <HashColon/Real.hpp>
#include <HashColon/Feline/GeoValues.hpp>

namespace HashColon::Feline::TrajectoryClustering
{
	std::vector<HashColon::Feline::XYList> UniformSampling(
		std::vector<HashColon::Feline::XYList>& trajlist,
		size_t SampleNumber);

	/*
	* TrajectoryDistanceMeasureBase
	* Base class for all trajectory distance/similarity measuring methods
	*/
	class TrajectoryDistanceMeasureBase
		: public HashColon::Clustering::DistanceMeasureBase<HashColon::Feline::XYList>
	{
	public:
		struct _Params
		{
			bool Enable_ReversedSequence;
		};

	protected:
		static inline _Params _cDefault;
		const _Params _c;

	public:
		static _Params& GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };
		static void Initialize(const std::string configFilePath = "");

		HashColon::Real Measure(
			const HashColon::Feline::XYList& a,
			const HashColon::Feline::XYList& b
		) const override;

	protected:
		virtual HashColon::Real Measure_core(
			const HashColon::Feline::XYList& a,
			const HashColon::Feline::XYList& b) const = 0;

		TrajectoryDistanceMeasureBase(HashColon::Clustering::DistanceMeasureType type, _Params params = _cDefault)
			: HashColon::Clustering::DistanceMeasureBase<HashColon::Feline::XYList>(type), _c(params)
		{};
	};

	/*
	* Hausdorff distance
	*/
	class Hausdorff : public TrajectoryDistanceMeasureBase
	{
	public:
		// Hausdorff distance is sequence-invariant,
		// therefore turns of reverse option
		Hausdorff(_Params params = _cDefault)
			: TrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance,
				{ false })
		{};

		const std::string GetMethodName() const override final { return "Hausdorff"; };

	protected:
		HashColon::Real Measure_core(
			const HashColon::Feline::XYList& a,
			const HashColon::Feline::XYList& b) const override final;
	};

	/*
	* Euclidean distance
	*/
	class Euclidean : public TrajectoryDistanceMeasureBase
	{
	public:
		Euclidean(_Params params = _cDefault)
			: TrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance,
				params)
		{};

		const std::string GetMethodName() const override final { return "Euclidean"; };

	protected:
		HashColon::Real Measure_core(
			const HashColon::Feline::XYList& a,
			const HashColon::Feline::XYList& b) const override final;
	};

	/*
	* Merge distance
	* Ismail, A., & Vigneron, A. (2015).
	* A New Trajectory Similarity Measure for GPS Data.
	* Proceedings of the 6th ACM SIGSPATIAL International Workshop on GeoStreaming - IWGS’15.
	* http://dx.doi.org/10.1145/2833165.2833173
	*/
	class Merge : public TrajectoryDistanceMeasureBase
	{
	public:
		Merge(_Params params = _cDefault)
			: TrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance,
				params)
		{};

		const std::string GetMethodName() const override final { return "Merge"; };

	protected:
		HashColon::Real Measure_core(
			const HashColon::Feline::XYList& a,
			const HashColon::Feline::XYList& b) const override final;
	};

	/*
	* LCSS
	* Vlachos, M., Kollios, G., & Gunopulos, D. (2002).
	* Discovering similar multidimensional trajectories.
	* Proceedings - International Conference on Data Engineering, 673–684.
	* https://doi.org/10.1109/ICDE.2002.994784
	*/
	class LCSS : public TrajectoryDistanceMeasureBase
	{
	public:
		struct _Params : public TrajectoryDistanceMeasureBase::_Params
		{
			HashColon::Real Epsilon;
			HashColon::Real Delta;
		};

	protected:
		static inline _Params _cDefault;
		const _Params _c;

	public:
		static void Initialize(const std::string configFilePath = "");

		LCSS() : TrajectoryDistanceMeasureBase(
			HashColon::Clustering::DistanceMeasureType::similarity),
			_c({ TrajectoryDistanceMeasureBase::_cDefault, _cDefault.Epsilon, _cDefault.Delta })
		{};

		LCSS(_Params params)
			: TrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::similarity, params),
			_c(params)
		{};

		const std::string GetMethodName() const override final { return "LCSS"; };

	protected:
		HashColon::Real Measure_core(
			const HashColon::Feline::XYList& a,
			const HashColon::Feline::XYList& b) const override final;
	};

	/*
	* Make trajectory to 1-dimensional vector. 
	* And get multi-dimensional euclidean distance. 
	* Use PCA to reduce dimension.
	* IMPORTANT!! Uniform sampling should be done for all trajectories.
	*/
	class ProjectedPCA : public TrajectoryDistanceMeasureBase
	{
	public:
		struct _Params : public TrajectoryDistanceMeasureBase::_Params
		{
			size_t PcaDimension;
			bool Enable_AutoPca;
		};

	protected:
		static inline _Params _cDefault;
		_Params _c;

		Eigen::MatrixXR _pca;

	public:
		static void Initialize(const std::string configFilePath = "");

		void RunPCA(const std::vector<HashColon::Feline::XYList>& trajlist);

		ProjectedPCA()
			: TrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance),
			_c({ TrajectoryDistanceMeasureBase::_cDefault, 
				_cDefault.PcaDimension, _cDefault.Enable_AutoPca})
		{};

		ProjectedPCA(_Params params)
			: TrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance, params),
			_c(params)
		{};

		const std::string GetMethodName() const override final { return "ProjectedPCA"; };

	protected:
		HashColon::Real Measure_core(
			const HashColon::Feline::XYList& a,
			const HashColon::Feline::XYList& b) const override final;
	};

	/*
	* Dynamic Time Warping
	*/
	class DynamicTimeWarping : public TrajectoryDistanceMeasureBase
	{
	public:
		DynamicTimeWarping(_Params params = _cDefault)
			: TrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance,
				params)
		{};

		const std::string GetMethodName() const override final { return "DynamicTimeWarping"; };

	protected:
		HashColon::Real Measure_core(
			const HashColon::Feline::XYList& a,
			const HashColon::Feline::XYList& b) const override final;
	};

	/*
	* Modified Hausdorff	
	* Something like LCSS + Hausdorff
	* IMPORTANT!! Uniform sampling should be done for all trajectories.
	* 
	* Atev, S., Masoud, O., & Papanikolopoulos, N. (2006). 
	* Learning traffic patterns at intersections by spectral clustering of motion trajectories. 
	* IEEE International Conference on Intelligent Robots and Systems, 4851–4856.
	* https://doi.org/10.1109/IROS.2006.282362
	*/
	class ModifiedHausdorff : public TrajectoryDistanceMeasureBase
	{
	public:
		struct _Params : public TrajectoryDistanceMeasureBase::_Params
		{
			HashColon::Real NeighborhoodWindowSize; // w
			HashColon::Real InlierPortion; // alpha
		};

	protected:
		static inline _Params _cDefault;
		_Params _c;

	public:
		static void Initialize(const std::string configFilePath = "");

		ModifiedHausdorff()
			: TrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance),
			_c({ TrajectoryDistanceMeasureBase::_cDefault,
				_cDefault.NeighborhoodWindowSize, _cDefault.InlierPortion })
		{};

		ModifiedHausdorff(_Params params)
			: TrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance, params),
			_c(params)
		{};

		const std::string GetMethodName() const override final { return "ModifiedHausdorff"; };

	protected:
		HashColon::Real Measure_core(
			const HashColon::Feline::XYList& a,
			const HashColon::Feline::XYList& b) const override final;
	};

	void Initialize_All_TrajectoryDistanceMeasure();
}
#endif