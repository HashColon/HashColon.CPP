#ifndef HASHCOLON_FELINE_TRAJECTORYCLUSTERING_DISTANCEMEASURE_WTHXTD_HPP
#define HASHCOLON_FELINE_TRAJECTORYCLUSTERING_DISTANCEMEASURE_WTHXTD_HPP

#include <iostream>

#include <Eigen/Eigen>
#include <vector>
#include <memory>
#include <HashColon/Core/Real.hpp>
#include <HashColon/Clustering/ClusteringBase.hpp>
#include <HashColon/Feline/Types/VoyageSimple.hpp>

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
		static inline _Params _cDefault;
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
		{
			/*using namespace std;
			cout << "MonteCarloDomainUnit: " << _cDefault.MonteCarloDomainUnit << ", " << _c.MonteCarloDomainUnit << endl;
			cout << "MonteCarloDomainSize: " << _cDefault.MonteCarloDomainSize << ", " << _c.MonteCarloDomainSize << endl;
			cout << "MonteCarloErrorEpsilon: " << _cDefault.MonteCarloErrorEpsilon << ", " << _c.MonteCarloErrorEpsilon << endl;
			cout << "Pf_XtdSigmaRatio: " << _cDefault.Pf_XtdSigmaRatio << ", " << _c.Pf_XtdSigmaRatio << endl;
			cout << "Coeff_JS: " << _cDefault.Coeff_JS << ", " << _c.Coeff_JS << endl;
			cout << "Coeff_WS: " << _cDefault.Coeff_WS << ", " << _c.Coeff_WS << endl;
			cout << "Coeff_PF: " << _cDefault.Coeff_PF << ", " << _c.Coeff_PF << endl;
			cout << "Coeff_Euclidean: " << _cDefault.Coeff_Euclidean << ", " << _c.Coeff_Euclidean << endl;			*/
		};

		DtwXtd_BlendedDistance(_Params params)
			: XtdTrajectoryDistanceMeasureBase(
				HashColon::Clustering::DistanceMeasureType::distance, params),
			_c(params)
		{
			/*using namespace std;
			cout << "MonteCarloDomainUnit: " << _cDefault.MonteCarloDomainUnit << ", " << _c.MonteCarloDomainUnit << endl;
			cout << "MonteCarloDomainSize: " << _cDefault.MonteCarloDomainSize << ", " << _c.MonteCarloDomainSize << endl;
			cout << "MonteCarloErrorEpsilon: " << _cDefault.MonteCarloErrorEpsilon << ", " << _c.MonteCarloErrorEpsilon << endl;
			cout << "Pf_XtdSigmaRatio: " << _cDefault.Pf_XtdSigmaRatio << ", " << _c.Pf_XtdSigmaRatio << endl;
			cout << "Coeff_JS: " << _cDefault.Coeff_JS << ", " << _c.Coeff_JS << endl;
			cout << "Coeff_WS: " << _cDefault.Coeff_WS << ", " << _c.Coeff_WS << endl;
			cout << "Coeff_PF: " << _cDefault.Coeff_PF << ", " << _c.Coeff_PF << endl;
			cout << "Coeff_Euclidean: " << _cDefault.Coeff_Euclidean << ", " << _c.Coeff_Euclidean << endl;*/
		};

		const std::string GetMethodName() const override final { return "DtwXtd_Blend"; };

	protected:
		virtual HashColon::Real Measure_core(
			const HashColon::Feline::XYXtdList& a,
			const HashColon::Feline::XYXtdList& b) const override final;
	};

	void Initialize_All_XtdTrajectoryDistanceMeasure();

}

#endif
