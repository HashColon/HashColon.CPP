#ifndef HASHCOLON_FELINE_TRAJECTORYCLUSTERING_DISTANCEMEASURE_HPP
#define HASHCOLON_FELINE_TRAJECTORYCLUSTERING_DISTANCEMEASURE_HPP

#include <vector>
#include <memory>
#include <HashColon/Helper/Real.hpp>
#include <HashColon/Clustering/ClusteringBase.hpp>
#include <HashColon/Feline/Types/VoyageSimple.hpp>

namespace HashColon::Feline::TrajectoryClustering::DistanceMeasure
{
	std::vector<HashColon::Feline::Types::Simple::XYList> UniformSampling(
		std::vector<HashColon::Feline::Types::Simple::XYList>& trajlist,
		size_t SampleNumber);	

	/*
	* TrajectoryDistanceMeasureBase
	* Base class for all trajectory distance/similarity measuring methods
	*/
	class TrajectoryDistanceMeasureBase
		: public HashColon::Clustering::DistanceMeasureBase<HashColon::Feline::Types::Simple::XYList>
	{
	public:
		struct _Params
		{
			bool Enable_ReversedSequence;
		};

	protected:
		static _Params _cDefault;
		const _Params _c;		

	public:
		static _Params& GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };
		static void Initialize(const std::string configFilePath = "");

		HashColon::Helper::Real Measure(
			const HashColon::Feline::Types::Simple::XYList& a,
			const HashColon::Feline::Types::Simple::XYList& b
		) const override;

	protected:
		virtual HashColon::Helper::Real Measure_core(
			const HashColon::Feline::Types::Simple::XYList& a,
			const HashColon::Feline::Types::Simple::XYList& b) const = 0;

		TrajectoryDistanceMeasureBase(HashColon::Clustering::DistanceMeasureType type, _Params params = _cDefault)
			: _c(params),
			HashColon::Clustering::DistanceMeasureBase<HashColon::Feline::Types::Simple::XYList>(type)
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
		HashColon::Helper::Real Measure_core(
			const HashColon::Feline::Types::Simple::XYList& a,
			const HashColon::Feline::Types::Simple::XYList& b) const override final;
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
		HashColon::Helper::Real Measure_core(
			const HashColon::Feline::Types::Simple::XYList& a,
			const HashColon::Feline::Types::Simple::XYList& b) const override final;
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
		HashColon::Helper::Real Measure_core(
			const HashColon::Feline::Types::Simple::XYList& a,
			const HashColon::Feline::Types::Simple::XYList& b) const override final;
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
			HashColon::Helper::Real Epsilon;
			HashColon::Helper::Real Delta;
		};

	protected:
		static _Params _cDefault;
		const _Params _c;

	public:
		static void Initialize(const std::string configFilePath = "");

		LCSS(_Params params = _cDefault)
			: TrajectoryDistanceMeasureBase(				
				HashColon::Clustering::DistanceMeasureType::distance, params),
			_c(params)
		{};

		const std::string GetMethodName() const override final { return "LCSS"; };

	protected:
		HashColon::Helper::Real Measure_core(
			const HashColon::Feline::Types::Simple::XYList& a,
			const HashColon::Feline::Types::Simple::XYList& b) const override final;
	};
}
#endif