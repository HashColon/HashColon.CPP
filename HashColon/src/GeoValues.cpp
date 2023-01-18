// HashColon config
#include <HashColon/HashColon_config.h>
// std libraries
#include <array>
#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <utility>
#include <vector>
// HashColon libraries
#include <HashColon/Array.hpp>
#include <HashColon/Exception.hpp>
#include <HashColon/Real.hpp>
#include <HashColon/SingletonCLI.hpp>
// header file for this source file
#include <HashColon/GeoValues.hpp>

using namespace std;
using namespace HashColon;

// define a const to ease computation
const Real PI180 = Constant::PI / 180.0;

// GeoValues: ShipIDKey, Position, Velocity, TimePoint, Duration, XTD & combined types
namespace HashColon
{
	Real Position::DistanceTo(const Position toPoint, GeoDistanceType type) const
	{
		return GeoDistance::Distance((*this), toPoint, type);
	}

	Position Position::MoveTo(const Real distanceMeter, const Degree a, GeoDistanceType type) const
	{
		return GeoDistance::MovePoint((*this), distanceMeter, a, type);
	}

	Degree Position::AngleTo(const Position toPoint, GeoDistanceType type) const
	{
		return GeoDistance::Angle((*this), toPoint, type);
	}

	Real Position::CrossTrackDistanceTo(const Position path_s, const Position path_e, GeoDistanceType type) const
	{
		return GeoDistance::CrossTrackDistance((*this), path_s, path_e, type);
	}

	Real Position::OnTrackDistanceTo(const Position path_s, const Position path_e, GeoDistanceType type) const
	{
		return GeoDistance::OnTrackDistance((*this), path_s, path_e, type);
	}

	XYT XYT::MoveTo(const Velocity vel, const Duration t, GeoDistanceType type) const
	{
		return GeoDistance::MovePoint((*this), vel, t, type);
	}

	Real XYT::SpeedTo(const XYT xyt, GeoDistanceType type) const
	{
		return GeoDistance::Speed((*this), xyt, type);
	}

	Velocity XYT::VelocityTo(const XYT xyt, GeoDistanceType type) const
	{
		return GeoDistance::VelocityBtwn((*this), xyt, type);
	}
}

// Coordinate systems: Cartesian(Rectequiv approximation)
namespace HashColon ::CoordSys
{
	HashColon::Real Cartesian::Distance(const Position A, const Position B)
	{
		using namespace HashColon::Vec2D;
		array<Real, 2> temp;
		temp = minus<Real>(B.dat, A.dat);
		temp[0] *= lonUnit();
		temp[1] *= latUnit();
		return abs<Real>(temp);
	}

	HashColon::Degree Cartesian::Angle(const Position A, const Position B)
	{
		using namespace HashColon::Vec2D;
		array<Real, 2> temp;
		temp = minus<Real>(B.dat, A.dat);
		temp[0] *= lonUnit();
		temp[1] *= latUnit();
		Real dist = abs<Real>(temp);

		return (
				   (temp[0] >= 0) ? acos(temp[1] / dist) : 2 * Constant::PI - acos(temp[1] / dist)) *
			   180 / Constant::PI;
	}

	HashColon::Degree Cartesian::Angle(const Position A, const Position B, const Position P)
	{
		Degree d1 = Angle(P, A);
		Degree d2 = Angle(P, B);
		return d2 - d1;
	}

	Position Cartesian::MovePoint(const Position A, const HashColon::Real d, const HashColon::Degree alpha)
	{
		Position re = A;
		re[0] += d * sin(alpha * Constant::PI / 180) / lonUnit();
		re[1] += d * cos(alpha * Constant::PI / 180) / latUnit();
		return re;
	}

	Position Cartesian::MovePoint(const Position A, const Velocity vel, const Duration t)
	{
		throw NotImplementedException;
	}

	XYT Cartesian::MovePoint(const XYT A, const Velocity vel, const Duration t)
	{
		throw NotImplementedException;
	}

	HashColon::Real Cartesian::CrossTrackDistance(const Position P, const Position path_S, const Position path_E)
	{
		using namespace HashColon::Vec2D;
		array<Real, 2> _P{{lonUnit() * P.longitude, latUnit() * P.latitude}};
		array<Real, 2> _S{{lonUnit() * path_S.longitude, latUnit() * path_S.latitude}};
		array<Real, 2> _E{{lonUnit() * path_E.longitude, latUnit() * path_E.latitude}};
		return PointToLineDistance(_S, _E, _P);
	}

	HashColon::Real Cartesian::OnTrackDistance(const Position P, const Position path_S, const Position path_E)
	{
		using namespace HashColon::Vec2D;
		array<Real, 2> _P{{lonUnit() * P.longitude, latUnit() * P.latitude}};
		array<Real, 2> _S{{lonUnit() * path_S.longitude, latUnit() * path_S.latitude}};
		array<Real, 2> _E{{lonUnit() * path_E.longitude, latUnit() * path_E.latitude}};

		return dot(minus(_P, _S), minus(_E, _S)) / abs(minus(_E, _S));
	}

	HashColon::Real Cartesian::Speed(const XYT A, const XYT B)
	{
		return Distance(A, B) / (Real)(A.TP - B.TP).count();
	}

	Velocity Cartesian::VelocityBtwn(const XYT A, const XYT B)
	{
		return {Speed(A, B), Angle(A, B)};
	}

	HashColon::Real Cartesian::Distance_atDesignatedLocation(const Position A, const Position B, const Position base)
	{
		using namespace HashColon::Vec2D;
		array<Real, 2> temp;
		temp = minus<Real>(B.dat, A.dat);
		temp[0] *= lonUnit(base);
		temp[1] *= latUnit(base);
		return abs<Real>(temp);
	}

	HashColon::Degree Cartesian::Angle_atDesignatedLocation(const Position A, const Position B, const Position base)
	{
		using namespace HashColon::Vec2D;
		array<Real, 2> temp;
		temp = minus<Real>(B.dat, A.dat);
		temp[0] *= lonUnit(base);
		temp[1] *= latUnit(base);
		Real dist = abs<Real>(temp);

		return (
				   (temp[0] >= 0) ? acos(temp[1] / dist) : 2 * Constant::PI - acos(temp[1] / dist)) *
			   180 / Constant::PI;
	}

	HashColon::Degree Cartesian::Angle_atDesignatedLocation(const Position A, const Position B, const Position P, const Position base)
	{
		Degree d1 = Angle_atDesignatedLocation(P, A, base);
		Degree d2 = Angle_atDesignatedLocation(P, B, base);
		return d2 - d1;
	}

	Position Cartesian::MovePoint_atDesignatedLocation(const Position A, const HashColon::Real d, const HashColon::Degree alpha, const Position base)
	{
		Position re = A;
		re[0] += d * sin(alpha * Constant::PI / 180) / lonUnit(base);
		re[1] += d * cos(alpha * Constant::PI / 180) / latUnit(base);
		return re;
	}

	Position Cartesian::MovePoint_atDesignatedLocation(const Position A, const Velocity vel, const Duration t, const Position base)
	{
		throw NotImplementedException;
	}

	XYT Cartesian::MovePoint_atDesignatedLocation(const XYT A, const Velocity vel, const Duration t, const Position base)
	{
		throw NotImplementedException;
	}

	HashColon::Real Cartesian::CrossTrackDistance_atDesignatedLocation(const Position P, const Position path_S, const Position path_E, const Position base)
	{
		using namespace HashColon::Vec2D;
		array<Real, 2> _P{{lonUnit(base) * P.longitude, latUnit(base) * P.latitude}};
		array<Real, 2> _S{{lonUnit(base) * path_S.longitude, latUnit(base) * path_S.latitude}};
		array<Real, 2> _E{{lonUnit(base) * path_E.longitude, latUnit(base) * path_E.latitude}};
		return PointToLineDistance(_S, _E, _P);
	}

	HashColon::Real Cartesian::OnTrackDistance_atDesignatedLocation(const Position P, const Position path_S, const Position path_E, const Position base)
	{
		using namespace HashColon::Vec2D;
		array<Real, 2> _P{{lonUnit(base) * P.longitude, latUnit(base) * P.latitude}};
		array<Real, 2> _S{{lonUnit(base) * path_S.longitude, latUnit(base) * path_S.latitude}};
		array<Real, 2> _E{{lonUnit(base) * path_E.longitude, latUnit(base) * path_E.latitude}};
		return (_E[0] - _S[0]) * (_P[0] - _S[0]) + (_E[1] - _S[1]) * (_P[1] - _S[1]);
	}

	HashColon::Real Cartesian::Speed_atDesignatedLocation(const XYT A, const XYT B, const Position base)
	{
		return Distance_atDesignatedLocation(A, B, base) / (Real)(A.TP - B.TP).count();
	}

	Velocity Cartesian::VelocityBtwn_atDesignatedLocation(const XYT A, const XYT B, const Position base)
	{
		return {Speed_atDesignatedLocation(A, B, base), Angle_atDesignatedLocation(A, B, base)};
	}
}

// Coordinate system: Haversine distnace(geodesic)
namespace HashColon ::CoordSys
{
	HashColon::Real Haversine::Distance(const Position A, const Position B)
	{
		// compute grand route using haversine formula
		using namespace std;
		using namespace HashColon;
		const Radian A_lat = A.latitude * PI180;
		const Radian B_lat = B.latitude * PI180;
		const Radian delta_lonhalf = ((B.longitude - A.longitude) / 2.0) * PI180;
		const Radian delta_lathalf = ((B.latitude - A.latitude) / 2.0) * PI180;
		const Real sinlat = sin(delta_lathalf);
		const Real sinlon = sin(delta_lonhalf);
		const Real a = sinlat * sinlat + cos(A_lat) * cos(B_lat) * sinlon * sinlon;
		return 2 * atan2(sqrt(a), sqrt(1 - a)) * EarthRadius::Val();
	}

	HashColon::Degree Haversine::Angle(const Position A, const Position B)
	{
		const Radian Alat = A.latitude * PI180;
		const Radian Alon = A.longitude * PI180;
		const Radian Blat = B.latitude * PI180;
		const Radian Blon = B.longitude * PI180;
		const Real y = sin(Blon - Alon) * cos(Blat);
		const Real x = cos(Alat) * sin(Blat) - sin(Alat) * cos(Blat) * cos(Blon - Alon);
		const Radian bearing = atan2(y, x);
		const Degree re = (Degree)bearing;
		return re;
	}

	HashColon::Degree Haversine::Angle(const Position A, const Position B, const Position P)
	{
		Degree d1 = Angle(P, A);
		Degree d2 = Angle(P, B);
		return d2 - d1;
	}

	Position Haversine::MovePoint(const Position A, const HashColon::Real d, const HashColon::Degree alpha)
	{
		const Radian delta = d / EarthRadius::Val();
		const Radian reLat = asin(sin(A.latitude * PI180) * cos(delta) + cos(A.latitude * PI180) * sin(delta) * cos(alpha * PI180));
		const Radian reLon = A.longitude * PI180 + std::atan2(sin(alpha * PI180) * sin(delta) * cos(A.latitude * PI180),
															  cos(delta) - sin(A.latitude * PI180) * sin(reLat));

		return {reLon / PI180, reLat / PI180};

		// Position re = A;
		//// compute latitude
		// const Radian deltaLat = d / EarthRadius::Val() * cos((Radian)alpha);
		// const Real reLat = re.latitude * PI180 + deltaLat;
		// if (abs(reLat) > (Constant::PI / 2.0))
		//	re[1] = (reLat > 0 ? Constant::PI - reLat : -Constant::PI - reLat) / PI180;
		// else
		//	re[1] = reLat / PI180;
		//// compute longitude
		// const Real deltaPsi = log(tan(re.latitude * PI180 / 2 + Constant::PI / 4) / tan(A.latitude * PI180 / 2 + Constant::PI / 4));
		// const Real q = abs(deltaPsi) > 1e-12 ? deltaLat / deltaPsi : cos(A.latitude * PI180);
		// re[0] += (d / EarthRadius::Val() * sin((Radian)alpha) / q) / PI180;

		// return re;
	}

	Position Haversine::MovePoint(const Position A, const Velocity vel, const Duration t)
	{
		throw NotImplementedException;
	}

	XYT Haversine::MovePoint(const XYT A, const Velocity vel, const Duration t)
	{
		throw NotImplementedException;
	}

	HashColon::Real Haversine::CrossTrackDistance(const Position P, const Position path_S, const Position path_E)
	{
		const Real adist_SP = Distance(path_S, P) / EarthRadius::Val(); // angular distance from S to P
		const Degree angle_SP = Angle(path_S, P);
		const Degree angle_SE = Angle(path_S, path_E);

		return EarthRadius::Val() * asin(sin(adist_SP) * sin((Radian)(angle_SP - angle_SE)));
	}

	HashColon::Real Haversine::OnTrackDistance(const Position P, const Position path_S, const Position path_E)
	{
		const Real adist_SP = Distance(path_S, P) / EarthRadius::Val();					 // angular distance from S to P
		const Real axtdist = CrossTrackDistance(P, path_S, path_E) / EarthRadius::Val(); // angular cross track distance

		return EarthRadius::Val() * acos(cos(adist_SP) / cos(axtdist));
	}

	HashColon::Real Haversine::Speed(const XYT A, const XYT B)
	{
		return Distance(A, B) / (Real)(A.TP - B.TP).count();
	}

	Velocity Haversine::VelocityBtwn(const XYT A, const XYT B)
	{
		return {Speed(A, B), Angle(A, B)};
	}
}

// GeoDistance
namespace HashColon
{
	// Caller functions

	HashColon::Real GeoDistance::Distance(const Position A, const Position B, GeoDistanceType type)
	{
		return _coordsys[type]->Distance(A, B);
	}

	HashColon::Degree GeoDistance::Angle(const Position A, const Position B, GeoDistanceType type)
	{
		return _coordsys[type]->Angle(A, B);
	}

	HashColon::Degree GeoDistance::Angle(const Position A, const Position B, const Position P, GeoDistanceType type)
	{
		return _coordsys[type]->Angle(A, B, P);
	}

	Position GeoDistance::MovePoint(const Position A, const HashColon::Real d, const HashColon::Degree alpha, GeoDistanceType type)
	{
		return _coordsys[type]->MovePoint(A, d, alpha);
	}

	Position GeoDistance::MovePoint(const Position A, const Velocity vel, const Duration t, GeoDistanceType type)
	{
		return _coordsys[type]->MovePoint(A, vel, t);
	}

	XYT GeoDistance::MovePoint(const XYT A, const Velocity vel, const Duration t, GeoDistanceType type)
	{
		return _coordsys[type]->MovePoint(A, vel, t);
	}

	HashColon::Real GeoDistance::CrossTrackDistance(const Position P, const Position path_S, const Position path_E, GeoDistanceType type)
	{
		return _coordsys[type]->CrossTrackDistance(P, path_S, path_E);
	}

	HashColon::Real GeoDistance::OnTrackDistance(const Position P, const Position path_S, const Position path_E, GeoDistanceType type)
	{
		return _coordsys[type]->OnTrackDistance(P, path_S, path_E);
	}

	HashColon::Real GeoDistance::Speed(const XYT A, const XYT B, GeoDistanceType type)
	{
		return _coordsys[type]->Speed(A, B);
	}

	Velocity GeoDistance::VelocityBtwn(const XYT A, const XYT B, GeoDistanceType type)
	{
		return _coordsys[type]->VelocityBtwn(A, B);
	}

	// Initializers

	void GeoDistance::SetBaseLocation(const Position baselocation)
	{
		cartesian.SetBase(baselocation);
	}

	void GeoDistance::SetDistanceMethod(GeoDistanceType type)
	{
		assert(type != DefaultDistance);
		if (type == DefaultDistance)
			throw Exception("Option DefaultDistance cannot be set to default distance.");
		_coordsys[DefaultDistance] = _coordsys[type];
	}
	void GeoDistance::Initialize(GeoDistanceType type, const Position baselocation)
	{
		SetDistanceMethod(type);
		SetBaseLocation(baselocation);
	}

	void GeoDistance::Initialize(const string configFilePath, const string configNamespace)
	{
		CLI::App *cli = SingletonCLI::GetInstance().GetCLI(configNamespace);

		// create help string for default distance type
		string allCoordSysNames = "";
		for (auto coordname : _coordsysName)
		{
			allCoordSysNames += (coordname.first + "/");
		}

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option_function<vector<Real>>(
			"--baseLocation",
			[](const vector<Real> &val)
			{
				SetBaseLocation({val[0], val[1]});
			},
			"Set base location for equirectangular approximation in format [lon, lat].")
			->envname(GetEnvName(configNamespace, "baseLocation"));

		cli->add_option_function<string>(
			"--defaultDistanceType",
			[](const string &val)
			{
				SetDistanceMethod(_coordsysName.at(val));
			},
			"Set default distance method. (" + allCoordSysNames + ")")
			->envname(GetEnvName(configNamespace, "defaultDistanceType"));
	}
}

// List of values: XYList, XYTList...
namespace HashColon
{
	// common functions
	namespace _common
	{
		template <typename TList>
		Real _GetLength(const TList &list, size_t sIndex, size_t eIndex)
		{
			// check validity
			if (eIndex >= list.size())
				eIndex = list.size() - 1;
			assert(sIndex < list.size());
			assert(sIndex <= eIndex);
			Real re = 0;
			for (size_t i = sIndex; i < eIndex; i++)
				re += GeoDistance::Distance(list[i], list[i + 1]);
			return re;
		}

		template <typename TList>
		vector<Real> _GetLengths(const TList &list)
		{
			// input list should have at least 1 item. if not, return empty vector
			vector<Real> uVec;
			if (list.size() <= 0)
				return uVec;

			uVec.resize(list.size());
			uVec[0] = 0;
			for (size_t i = 1; i < list.size(); i++)
				uVec[i] = uVec[i - 1] + GeoDistance::Distance(list[i - 1], list[i]);
			return uVec;
		}

		template <typename TList>
		TList _GetLengthSampled(const TList &list, const vector<Real> &lengthParams)
		{
			// input list should have at least 1 item && length Params should have at least 1 item
			// if not, return empty vector
			TList re;
			if (lengthParams.size() <= 0 || list.size() <= 0)
				return re;

			re.resize(lengthParams.size());
			vector<Real> u1Vec = _GetLengths(list);	  // parameter u of original list
			const vector<Real> &u2Vec = lengthParams; // parameter u of new list

			size_t i1 = 0;
			for (size_t i2 = 0; i2 < u2Vec.size(); i2++)
			{
				// assert if u2Vec is in increasing order
				assert(i2 <= 0 ? true : u2Vec[i2 - 1] < u2Vec[i2]);

				const Real &u2 = u2Vec[i2];
				// u1Vec[i1] /* u1 */ <= u2 < u1Vec[i1+1] /* u1max */
				for (; u2 > u1Vec[i1 + 1] && i1 < u1Vec.size(); i1++)
					;
				const Real &u1 = u1Vec[i1];

				// compute point at i2
				re[i2] = list[i1];
				if (!(u1 == u2 || i1 >= u1Vec.size() - 1))
				{
					auto tmpPt = GeoDistance::MovePoint(list[i1], u2 - u1, GeoDistance::Angle(list[i1], list[i1 + 1]));
					if constexpr (std::is_same<Position, typename TList::value_type>::value)
						re[i2] = tmpPt;
					else
						re[i2].Pos = tmpPt;
				}
			}
			return re;
		}

		template <typename TList>
		TList _GetUniformLengthSampled(const TList &list, size_t sizeN)
		{
			assert(sizeN >= 2);
			vector<Real> lengthParams(sizeN, 0.0);
			if (sizeN >= 2)
			{
				// build length params
				const Real len = _GetLength(list, 0, list.size() - 1);
				const Real step = len / ((Real)sizeN - 1.0);
				for (size_t v = 0; v < sizeN - 1; v++)
					lengthParams[v] = (Real)v * step;
				lengthParams[sizeN - 1] = len;
			}
			return _GetLengthSampled(list, lengthParams);
		}

		template <typename TList>
		Duration _GetElapsedTime(const TList &list, size_t sIndex, size_t eIndex)
		{
			TimePoint sTP = list[sIndex];
			TimePoint eTP = list[eIndex];
			return eTP - sTP;
		}

		template <typename TList>
		vector<Duration> _GetElapsedTimes(const TList &list)
		{
			vector<Duration> re(list.size());
			for (size_t i = 0; i < list.size(); i++)
				re[i] = _GetElapsedTime(list, 0, i);
			return re;
		}

		template <typename TList>
		TList _GetTimeSampled(const TList &list, const vector<Duration> &timeParams)
		{
			// input list should have at least 1 item && length Params should have at least 1 item
			// if not, return empty vector
			TList re;
			if (timeParams.size() <= 0 || list.size() <= 0)
				return re;

			re.resize(timeParams.size());
			vector<Duration> u1Vec = _GetElapsedTimes(list); // parameter u of original list
			const vector<Duration> &u2Vec = timeParams;		 // parameter u of new list

			size_t i1 = 0;
			for (size_t i2 = 0; i2 < u2Vec.size(); i2++)
			{
				// assert if u2Vec is in increasing order
				assert(i2 <= 0 ? true : u2Vec[i2 - 1] < u2Vec[i2]);

				const Duration &u2 = u2Vec[i2];
				// u1Vec[i1] /* u1 */ <= u2 < u1Vec[i1+1] /* u1max */
				for (; u2 > u1Vec[i1 + 1] && i1 < u1Vec.size(); i1++)
					;
				const Duration u1 = u1Vec[i1];

				// compute point at i2
				re[i2] = list[i1];
				if (!(u1 == u2 || i1 >= u1Vec.size() - 1))
				{
					auto tmpPt = GeoDistance::MovePoint((XY)list[i1], GeoDistance::VelocityBtwn(list[i1], list[i1 + 1]), u2 - u1);
					if constexpr (std::is_same<Position, typename TList::value_type>::value)
						re[i2] = tmpPt;
					else
						re[i2].Pos = tmpPt;
				}
			}
			return re;
		}

		template <typename TList>
		TList _GetUniformTimeSampled(const TList &list, size_t sizeN)
		{
			assert(sizeN >= 2);
			vector<Duration> timeParams(sizeN);
			if (sizeN >= 2)
			{
				// build time parmas
				const Duration step = _GetElapsedTime(list, 0, list.size() - 1) / (sizeN - 1);
				for (size_t v = 0; v < sizeN; v++)
					timeParams[v] = v * step;
			}
			return _GetTimeSampled(list, timeParams);
		}

		template <typename TList>
		TList _GetReversed(const TList &list)
		{
			TList re;
			re.resize(list.size());
			reverse_copy(list.begin(), list.end(), re.begin());
			return re;
		}

		template <typename fromTList, typename toTList>
		toTList _ConvertList(const fromTList &list)
		{
			toTList re;
			re.clear();
			for (const auto &it : list)
				re.push_back(it);
			return re;
		}
	}

	using namespace _common;

	// XYList
	Real XYList::GetLength(size_t sIndex, size_t eIndex) const { return _GetLength((*this), sIndex, eIndex); }
	vector<Real> XYList::GetLengths() const { return _GetLengths(*this); }
	XYList XYList::GetLengthSampled(const vector<Real> &lengthParams) const { return _GetLengthSampled((*this), lengthParams); }
	XYList XYList::GetUniformLengthSampled(size_t sizeN) const { return _GetUniformLengthSampled((*this), sizeN); }
	XYList XYList::GetReversed() const { return _GetReversed((*this)); }

	// XYXtdList
	Real XYXtdList::GetLength(size_t sIndex, size_t eIndex) const { return _GetLength((*this), sIndex, eIndex); }
	vector<Real> XYXtdList::GetLengths() const { return _GetLengths(*this); }
	XYXtdList XYXtdList::GetLengthSampled(const vector<Real> &lengthParams) const { return _GetLengthSampled((*this), lengthParams); }
	XYXtdList XYXtdList::GetUniformLengthSampled(size_t sizeN) const { return _GetUniformLengthSampled((*this), sizeN); }
	XYXtdList XYXtdList::GetReversed() const { return _GetReversed(*this); }
	XYList XYXtdList::ToXYList() const { return _ConvertList<XYXtdList, XYList>(*this); }

	// XYTList
	Real XYTList::GetLength(size_t sIndex, size_t eIndex) const { return _GetLength((*this), sIndex, eIndex); }
	vector<Real> XYTList::GetLengths() const { return _GetLengths(*this); }
	XYTList XYTList::GetLengthSampled(const vector<Real> &lengthParams) const { return _GetLengthSampled((*this), lengthParams); }
	XYTList XYTList::GetUniformLengthSampled(size_t sizeN) const { return _GetUniformLengthSampled((*this), sizeN); }
	Duration XYTList::GetElapsedTime(size_t sIndex, size_t eIndex) const { return _GetElapsedTime((*this), sIndex, eIndex); }
	vector<Duration> XYTList::GetElapsedTimes() const { return _GetElapsedTimes((*this)); };
	XYTList XYTList::GetTimeSampled(const vector<Duration> &timeParams) const { return _GetTimeSampled((*this), timeParams); }
	XYTList XYTList::GetUniformTimeSampled(size_t sizeN) const { return _GetUniformTimeSampled((*this), sizeN); }
	XYTList XYTList::GetReversed() const { return _GetReversed(*this); }
	XYList XYTList::ToXYList() const { return _ConvertList<XYTList, XYList>(*this); }

	// XYVVaTList
	Real XYVVaTList::GetLength(size_t sIndex, size_t eIndex) const { return _GetLength((*this), sIndex, eIndex); }
	vector<Real> XYVVaTList::GetLengths() const { return _GetLengths(*this); }
	XYVVaTList XYVVaTList::GetLengthSampled(const vector<Real> &lengthParams) const { return _GetLengthSampled((*this), lengthParams); }
	XYVVaTList XYVVaTList::GetUniformLengthSampled(size_t sizeN) const { return _GetUniformLengthSampled((*this), sizeN); }
	Duration XYVVaTList::GetElapsedTime(size_t sIndex, size_t eIndex) const { return _GetElapsedTime((*this), sIndex, eIndex); }
	vector<Duration> XYVVaTList::GetElapsedTimes() const { return _GetElapsedTimes((*this)); };
	XYVVaTList XYVVaTList::GetTimeSampled(const vector<Duration> &timeParams) const { return _GetTimeSampled((*this), timeParams); }
	XYVVaTList XYVVaTList::GetUniformTimeSampled(size_t sizeN) const { return _GetUniformTimeSampled((*this), sizeN); }
	XYVVaTList XYVVaTList::GetReversed() const { return _GetReversed(*this); }
	XYList XYVVaTList::ToXYList() const { return _ConvertList<XYVVaTList, XYList>(*this); }
	XYTList XYVVaTList::ToXYTList() const { return _ConvertList<XYVVaTList, XYTList>(*this); }

	// XYVVaXtdTList
	Real XYVVaXtdTList::GetLength(size_t sIndex, size_t eIndex) const { return _GetLength((*this), sIndex, eIndex); }
	vector<Real> XYVVaXtdTList::GetLengths() const { return _GetLengths(*this); }
	XYVVaXtdTList XYVVaXtdTList::GetLengthSampled(const vector<Real> &lengthParams) const { return _GetLengthSampled((*this), lengthParams); }
	XYVVaXtdTList XYVVaXtdTList::GetUniformLengthSampled(size_t sizeN) const { return _GetUniformLengthSampled((*this), sizeN); }
	Duration XYVVaXtdTList::GetElapsedTime(size_t sIndex, size_t eIndex) const { return _GetElapsedTime((*this), sIndex, eIndex); }
	vector<Duration> XYVVaXtdTList::GetElapsedTimes() const { return _GetElapsedTimes((*this)); };
	XYVVaXtdTList XYVVaXtdTList::GetTimeSampled(const vector<Duration> &timeParams) const { return _GetTimeSampled((*this), timeParams); }
	XYVVaXtdTList XYVVaXtdTList::GetUniformTimeSampled(size_t sizeN) const { return _GetUniformTimeSampled((*this), sizeN); }
	XYVVaXtdTList XYVVaXtdTList::GetReversed() const { return _GetReversed(*this); }
	XYList XYVVaXtdTList::ToXYList() const { return _ConvertList<XYVVaXtdTList, XYList>(*this); }
	XYXtdList XYVVaXtdTList::ToXYXtdList() const { return _ConvertList<XYVVaXtdTList, XYXtdList>(*this); }
	XYTList XYVVaXtdTList::ToXYTList() const { return _ConvertList<XYVVaXtdTList, XYTList>(*this); }
	XYVVaTList XYVVaXtdTList::ToXYVVaTList() const { return _ConvertList<XYVVaXtdTList, XYVVaTList>(*this); }
}
