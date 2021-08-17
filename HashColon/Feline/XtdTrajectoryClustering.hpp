#ifndef HASHCOLON_FELINE_XTDTRAJECTORYCLUSTERING_HPP
#define HASHCOLON_FELINE_XTDTRAJECTORYCLUSTERING_HPP

// std libraries
#include <memory>
#include <vector>
#include <Eigen/Eigen>
// HashColon libraries
#include <HashColon/Clustering.hpp>
#include <HashColon/Real.hpp>
#include <HashColon/Feline/ValueTypes.hpp>

// XTD distance btwn waypoints
namespace HashColon::Feline::XtdTrajectoryClustering
{
	/* JS-divergence distance */
	struct _JSDivergenceOption
	{
		HashColon::Real domainUnit;
		HashColon::Real domainSize;
		HashColon::Real errorEpsilon;
	};

	const _JSDivergenceOption _cDefault_JSDivergence = {
		1.0, 3.0, 1e-6
	};;

	HashColon::Real JSDivergenceDistance(
		HashColon::Feline::XYXtd a, HashColon::Degree aDir,
		HashColon::Feline::XYXtd b, HashColon::Degree bDir,
		_JSDivergenceOption options = _cDefault_JSDivergence);

	/* Wasserstein distance */
	struct _WassersteinOption
	{
		HashColon::Real domainUnit;
		HashColon::Real domainSize;
		HashColon::Real errorEpsilon;
	};

	const _WassersteinOption _cDefault_Wasserstein = {
		1.0, 3.0, 1e-6
	};

	HashColon::Real WassersteinDistance(
		HashColon::Feline::XYXtd a, HashColon::Degree aDir,
		HashColon::Feline::XYXtd b, HashColon::Degree bDir,
		_WassersteinOption options = _cDefault_Wasserstein);

	/* PF distance */
	struct _PFDistanceOption
	{
		HashColon::Real XtdSigmaRatio;
	};

	const _PFDistanceOption _cDefault_PFDistance = { 3.0 };

	HashColon::Real PFDistance(
		HashColon::Feline::XYXtd a, HashColon::Degree aDir,
		HashColon::Feline::XYXtd b, HashColon::Degree bDir,
		_PFDistanceOption options = _cDefault_PFDistance);

}

// XTD distance metric btwn trajectories
namespace HashColon::Feline::XtdTrajectoryClustering
{
	std::vector<HashColon::Feline::XYXtdList> UniformSampling(
		std::vector<HashColon::Feline::XYXtdList>& trajlist,
		size_t SampleNumber);

	/*
	* XtdTrajectoryDistanceMeasureBase
	* Base class for trajectory with Xtd distance/similarity measuring methods
	*/
	class XtdTrajectoryDistanceMeasureBase
		: public HashColon::Clustering::DistanceMeasureBase<HashColon::Feline::XYXtdList>
	{
	public:
		struct _Params
		{
			bool Enable_ReversedSequence;				
		};

	protected:
		inline static _Params _cDefault;
		const _Params _c;

	public:
		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };
		static void Initialize(const std::string configFilePath = "");

		HashColon::Real Measure(
			const HashColon::Feline::XYXtdList& a,
			const HashColon::Feline::XYXtdList& b
		) const override;

	protected:
		virtual HashColon::Real Measure_core(
			const HashColon::Feline::XYXtdList& a,
			const HashColon::Feline::XYXtdList& b) const = 0;

		XtdTrajectoryDistanceMeasureBase(
			HashColon::Clustering::DistanceMeasureType type, _Params params = _cDefault)
			: HashColon::Clustering::DistanceMeasureBase<HashColon::Feline::XYXtdList>(type),
			_c(params)
		{};
	};	

	class DtwXtd : public XtdTrajectoryDistanceMeasureBase
	{
	public:
		DtwXtd(_Params params = _cDefault) : 
			XtdTrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance)
		{};

		const std::string GetMethodName() const override final { return "DtwXtd"; };

	protected:
		HashColon::Real Measure_core(
			const HashColon::Feline::XYXtdList& a,
			const HashColon::Feline::XYXtdList& b) const override final;		
	};

	/*
	* Dynamic Time Warping: using JS divergence
	*/
	class DtwXtd_usingJSDivergence : public XtdTrajectoryDistanceMeasureBase
	{
	public:
		struct _Params : public XtdTrajectoryDistanceMeasureBase::_Params
		{
			HashColon::Real MonteCarloDomainUnit;
			HashColon::Real MonteCarloDomainSize;
			HashColon::Real MonteCarloErrorEpsilon;
		};

	protected:
		static inline _Params _cDefault;
		_Params _c;
		
	public:
		static void Initialize(const std::string configFilePath = "");

		DtwXtd_usingJSDivergence()
			: XtdTrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance),
			_c({
				XtdTrajectoryDistanceMeasureBase::_cDefault,
				_cDefault.MonteCarloDomainUnit, 
				_cDefault.MonteCarloDomainSize, 
				_cDefault.MonteCarloErrorEpsilon })
		{};

		DtwXtd_usingJSDivergence(_Params params)
			: XtdTrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance, params),
			_c(params)
		{};

		const std::string GetMethodName() const override final { return "DtwXtd_JS"; };

	protected:
		virtual HashColon::Real Measure_core(
			const HashColon::Feline::XYXtdList& a,
			const HashColon::Feline::XYXtdList& b) const override final;
	};

	/*
	* Dynamic Time Warping using WS divergence
	*/
	class DtwXtd_usingWasserstein : public XtdTrajectoryDistanceMeasureBase
	{
	public:
		struct _Params : public XtdTrajectoryDistanceMeasureBase::_Params
		{
			HashColon::Real MonteCarloDomainUnit;
			HashColon::Real MonteCarloDomainSize;
			HashColon::Real MonteCarloErrorEpsilon;
		};

	protected:
		static inline _Params _cDefault;
		_Params _c;

	public:
		static void Initialize(const std::string configFilePath = "");

		DtwXtd_usingWasserstein()
			: XtdTrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance),
			_c({
				XtdTrajectoryDistanceMeasureBase::_cDefault,
				_cDefault.MonteCarloDomainUnit,
				_cDefault.MonteCarloDomainSize,
				_cDefault.MonteCarloErrorEpsilon })
		{};

		DtwXtd_usingWasserstein(_Params params)
			: XtdTrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance, params),
			_c(params)
		{};

		const std::string GetMethodName() const override final { return "DtwXtd_EMD"; };

	protected:
		virtual HashColon::Real Measure_core(
			const HashColon::Feline::XYXtdList& a,
			const HashColon::Feline::XYXtdList& b) const override final;
	};
	
	/*
	* Dynamic Time Warping using point-to-point distance as 
	* JS divergence & PF distance blended function.
	*/
	class DtwXtd_BlendedDistance : public XtdTrajectoryDistanceMeasureBase
	{	
	public:
		struct _Params : public XtdTrajectoryDistanceMeasureBase::_Params
		{
			HashColon::Real MonteCarloDomainUnit;
			HashColon::Real MonteCarloDomainSize;
			HashColon::Real MonteCarloErrorEpsilon;
			HashColon::Real Pf_XtdSigmaRatio;
			
			HashColon::Real Coeff_JS;
			HashColon::Real Coeff_WS;
			HashColon::Real Coeff_PF;
			HashColon::Real Coeff_Euclidean;
		};
	protected:
		static inline _Params _cDefault;
		_Params _c;

	public:
		static void Initialize(const std::string configFilePath = "");

		DtwXtd_BlendedDistance()
			: XtdTrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance), 
			_c({
				XtdTrajectoryDistanceMeasureBase::_cDefault,
				_cDefault.MonteCarloDomainUnit,	_cDefault.MonteCarloDomainSize, _cDefault.MonteCarloErrorEpsilon,
				_cDefault.Pf_XtdSigmaRatio,
				_cDefault.Coeff_JS, _cDefault.Coeff_WS, _cDefault.Coeff_PF, _cDefault.Coeff_Euclidean })
		{};

		DtwXtd_BlendedDistance(_Params params)
			: XtdTrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance, params),
			_c(params)
		{};

		const std::string GetMethodName() const override final { return "DtwXtd_Blend"; };

	protected:
		virtual HashColon::Real Measure_core(
			const HashColon::Feline::XYXtdList& a,
			const HashColon::Feline::XYXtdList& b) const override final;
	};

	void Initialize_All_XtdTrajectoryDistanceMeasure();
}

#endif
