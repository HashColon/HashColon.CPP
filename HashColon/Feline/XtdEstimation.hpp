#ifndef HASHCOLON_FELINE_TRAJECTORYXTDESTIMATION
#define HASHCOLON_FELINE_TRAJECTORYXTDESTIMATION

// std libraries
#include <vector>
// HashColon libraries
#include <HashColon/Exception.hpp>
#include <HashColon/Real.hpp>
#include <HashColon/Feline/GeoData.hpp>
#include <HashColon/Feline/GeoValues.hpp>

namespace HashColon::Feline
{
	class CATZOC
	{
	public:
		enum CATZOC_CLASS
		{
			CATZOC_A1,
			CATZOC_A2,
			CATZOC_B,
			CATZOC_C,
			CATZOC_D
		};
		static CATZOC_CLASS GetValue(HashColon::Feline::Position p);
	};

	class NavAreaType
	{
	public:
		HASHCOLON_CLASS_EXCEPTION_DEFINITION(NavAreaType);

		enum NavArea_CLASS
		{
			NavArea_HarborNConfinedWaters,
			NavArea_Coastal,
			NavArea_OpenSea
		};
		static NavArea_CLASS GetValue(HashColon::Feline::Position p);
	};

	class XTDEstimation
	{
	public:
		struct _Params
		{
			std::vector<std::vector<HashColon::Real>> CatzocDistanceTable;
			std::vector<std::vector<HashColon::Real>> NavigationalSafetyAllowanceTable;
			HashColon::Real PositionPrecision;
			HashColon::Real DistanceMargin_D_coast;
			HashColon::Real DraughtMargin_D_coast;
		};

		HASHCOLON_CLASS_EXCEPTION_DEFINITION(XTDEstimation);

	protected:
		static inline _Params _cDefault;
		_Params _c;
		size_t _threadIdx;

	protected:
		HashColon::Feline::XTD _Get_d_zoc(HashColon::Feline::Position pos) const;
		HashColon::Feline::XTD _Get_d_b(HashColon::Real beam) const;
		HashColon::Feline::XTD _Get_d_pos() const;
		HashColon::Feline::XTD _Get_d_nsa(HashColon::Feline::Position pos) const;
		HashColon::Feline::XTD _Get_d_vsa(HashColon::Real loa, HashColon::Degree turnAngle) const;
		HashColon::Feline::XTD _Get_d_coast(
			HashColon::Feline::Position pos, HashColon::Feline::Position legEnd,
			HashColon::Feline::XTD searchLimit, HashColon::Real beam, HashColon::Real draught) const;

	public:
		static void Initialize(const std::string configFilePath = "");
		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };

		XTDEstimation(size_t threadidx = 0, _Params params = _cDefault)
			: _c(_cDefault), _threadIdx(threadidx){};

		XTD EstimateXTD(
			HashColon::Feline::Position pos, HashColon::Feline::Position legEnd,
			HashColon::Real loa, HashColon::Real beam, HashColon::Real draught,
			HashColon::Degree turnAngle) const;

		HashColon::Feline::XYXtdList EstimateXTD(
			HashColon::Feline::XYList waypoints,
			HashColon::Real loa, HashColon::Real beam, HashColon::Real draught) const;
	};
}

#endif
