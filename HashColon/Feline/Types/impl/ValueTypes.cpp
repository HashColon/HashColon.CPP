#include <iomanip>
#include <iosfwd>
#include <sstream>
#include <ctime>
#include <mutex>
#include <HashColon/Helper/SimpleVector.hpp>
#include <HashColon/Core/Real.hpp>
#include <HashColon/Core/Exception.hpp>
#include <HashColon/Feline/Types/ValueTypes.hpp>

using namespace std;
using namespace HashColon::Helper;

namespace HashColon::Feline
{
	mutex ctime_mx;
}

namespace HashColon::Feline
{
	// defined as extern in ValueTypes.hpp
	Real _northPole = 90.0;
	Real _southPole = -90.0;

	// These values are defined as constexpr in ValueTypes.hpp
	//const Real _LonUnitDist = 88.74 * 1000;  // 88.74km / degree
	//const Real _LatUnitDist = 109.96 * 1000; // 109.96km / degree	

	//{ TimePoint functions
	void TimePoint::fromString(const string formatStr, string datetimeStr)
	{
		tm temp_tm = { 0 };
		stringstream ss(datetimeStr.c_str());
		ss >> std::get_time(&temp_tm, formatStr.c_str());

		// check if the datetimestr satisfies the format
		if (ss.fail() || !ss)
			throw out_of_range("datetime string out of range ( HashColon::Feline::Types::Timepoint::fromString() ).");
		else
		{
			std::time_t temptime_t;

			// unfortunately, mktime and localtime from ctime is not threadsafe.
			// therefore a lock should be provided.
			// refer to the following link for more information about tregedic behavior of mktime & localtime
			//https://stackoverflow.com/questions/16575029/localtime-not-thread-safe-but-okay-to-call-in-only-one-thread
			{
				std::lock_guard<std::mutex> lock_mx(ctime_mx);
				temptime_t = std::mktime(&temp_tm);
			}
			auto temp_tp = chrono::system_clock::from_time_t(temptime_t);
			(*this) = temp_tp;
		}
	}

	string TimePoint::toString(const string formatStr) const
	{
		time_t this_C = chrono::system_clock::to_time_t(*this);
		stringstream ss;

		// unfortunately, mktime and localtime from ctime is not threadsafe.
		// therefore a lock should be provided.
		// refer to the following link for more information about tregedic behavior of mktime & localtime
		//https://stackoverflow.com/questions/16575029/localtime-not-thread-safe-but-okay-to-call-in-only-one-thread
		{
			lock_guard<std::mutex> lock_mx(ctime_mx);
			ss << put_time(std::localtime(&this_C), formatStr.c_str());
		}
		return ss.str();
	}
}

namespace HashColon::Feline::_hidden
{
	bool operator==(const _Position& lhs, const _Position& rhs)
	{
		return lhs.dat[0] == rhs.dat[0] && lhs.dat[1] == rhs.dat[1];
	}
	bool operator!=(const _Position& lhs, const _Position& rhs)
	{
		return !(lhs == rhs);
	}

	bool operator==(const _Velocity& lhs, const _Velocity& rhs)
	{
		return lhs.dat[0] == rhs.dat[0] && lhs.dat[1] == rhs.dat[1];
	}
	bool operator!=(const _Velocity& lhs, const _Velocity& rhs)
	{
		return !(lhs == rhs);
	}

	bool operator==(const _XTD& lhs, const _XTD& rhs)
	{
		return lhs.dat[0] == rhs.dat[0] && lhs.dat[1] == rhs.dat[1];
	}
	bool operator!=(const _XTD& lhs, const _XTD& rhs)
	{
		return !(lhs == rhs);
	}
}

namespace HashColon::Feline
{
	Real Distance(const Position& A, const Position& B)
	{
	#ifdef Feline_DISTANCE_USING_GRANDROUTE
		return Distance_GrandRoute(A, B);
	#else
		return Distance_CartesianDistance(A, B);
	#endif
	}

	Real Distance_CartesianDistance(const Position& A, const Position& B)
	{
		{
			using namespace HashColon::Helper::Vec2D;
			std::array<Real, 2> temp;
			temp = minus<Real>(B.dat, A.dat);
			temp[0] *= _LonUnitDist;
			temp[1] *= _LatUnitDist;
			return abs<Real>(temp);
		}
	}

	Real Distance_GrandRoute(const Position& A, const Position& B)
	{
		throw NotImplementedException;
	}

	Degree Angle(const Position& A, const Position& B)
	{
	#ifdef Feline_DISTANCE_USING_GRANDROUTE
		return Angle_GrandRoute(A, B);
	#else
		return Angle_CartesianDistance(A, B);
	#endif
	}

	Degree Angle_CartesianDistance(const Position& A, const Position& B)
	{
		using namespace HashColon::Helper::Vec2D;
		std::array<Real, 2> temp;
		temp = minus<Real>(B.dat, A.dat);
		temp[0] *= _LonUnitDist;
		temp[1] *= _LatUnitDist;
		Real dist = abs<Real>(temp);

		return (
			(temp[0] >= 0) ?
			std::acos(temp[1] / dist) :
			2 * M_PI - std::acos(temp[1] / dist)
			) * 180 / M_PI;
	}

	Degree Angle_GrandRoute(const Position& A, const Position& B)
	{
		throw NotImplementedException;
	}

	Degree Angle(const Position& A, const Position& B, const Position& P)
	{
	#ifdef Feline_DISTANCE_USING_GRANDROUTE
		return Angle_GrandRoute(A, B, P);
	#else
		return Angle_CartesianDistance(A, B, P);
	#endif
	}

	Degree Angle_CartesianDistance(const Position& A, const Position& B, const Position& P)
	{
		Degree d1 = Angle_CartesianDistance(P, A);
		Degree d2 = Angle_CartesianDistance(P, B);

		return d2 - d1;
	}

	Degree Angle_GrandRoute(const Position& A, const Position& B, const Position& P)
	{
		throw NotImplementedException;
	}

	Position MovePoint(Position A, Real d, Degree alpha)
	{
	#ifdef Feline_DISTANCE_USING_GRANDROUTE
		return MovePoint_GrandRoute(A, d, alpha);
	#else
		return MovePoint_CartesianDistance(A, d, alpha);
	#endif
	}

	Position MovePoint_CartesianDistance(const Position& A, const Real& d, const Degree& alpha)
	{
		Position re = A;

		re.longitude += d * sin(alpha * M_PI / 180) / _LonUnitDist;
		re.latitude += d * cos(alpha * M_PI / 180) / _LatUnitDist;

		return re;
	}

	Position MovePoint_GrandRoute(const Position& A, const Real& d, const Degree& alpha)	
	{
		throw NotImplementedException;
	}

	Real CrossTrackDistance(const Position& P, const Position& path_S, const Position& path_E)
	{
	#ifdef Feline_DISTANCE_USING_GRANDROUTE
		return CrossTrackDistance_GrandRoute(P, path_S, path_E);
	#else
		return CrossTrackDistance_CartesianDistance(P, path_S, path_E);
	#endif
	}

	Real CrossTrackDistance_CartesianDistance(const Position& P, const Position& path_S, const Position& path_E)
	{
		using namespace HashColon::Helper::Vec2D;
		std::array<Real, 2> _P{ { _LonUnitDist * P.longitude, _LatUnitDist * P.latitude} };
		std::array<Real, 2> _S{ { _LonUnitDist * path_S.longitude, _LatUnitDist * path_S.latitude} };
		std::array<Real, 2> _E{ { _LonUnitDist * path_E.longitude, _LatUnitDist * path_E.latitude} };
		return PointToLineDistance(_S, _E, _P);
	}

	Real CrossTrackDistance_GrandRoute(const Position& P, const Position& path_S, const Position& path_E)
	{
		throw NotImplementedException;
	}
}

namespace HashColon::Feline
{	
	Real Position::DistanceTo_usingCartesianDistance(Position toPoint) const
	{
		return Distance_CartesianDistance((*this), toPoint);
	}

	Real Position::DistanceTo_usingGrandRoute(Position toPoint) const
	{
		throw NotImplementedException;
	}

	Position Position::MoveTo_usingCartesianDistance(Degree a, Real distanceMeter) const
	{
		return MovePoint_CartesianDistance((*this), distanceMeter, a);
	}

	Position Position::MoveTo_usingGrandRoute(Degree a, Real distanceMeter) const
	{
		throw NotImplementedException;
	}

	Degree Position::AngleTo_usingCartesianDistance(Position toPoint) const
	{
		return Angle_CartesianDistance((*this), toPoint);
	}

	Degree Position::AngleTo_usingGrandRoute(Position toPoint) const
	{
		throw NotImplementedException;
	}

	Real Position::CrossTrackDistanceTo_usingCartesianDistance(Position path_s, Position path_e)
	{
		return CrossTrackDistance_CartesianDistance((*this), path_s, path_e);
	}

	Real Position::CrossTrackDistanceTo_usingGrandRoute(Position path_s, Position path_e)
	{
		throw NotImplementedException;
	}

}



