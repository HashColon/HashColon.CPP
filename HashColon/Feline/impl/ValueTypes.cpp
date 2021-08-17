// HashColon config
#include <HashColon/HashColon_config.h>
// std libraries
#include <array>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iosfwd>
#include <mutex>
#include <sstream>
#include <utility>
// HashColon libraries
#include <HashColon/Array.hpp>
#include <HashColon/Exception.hpp>
#include <HashColon/Real.hpp>
// header file for this source file
#include <HashColon/Feline/ValueTypes.hpp>

using namespace std;
using namespace HashColon;

// TimeStamp
namespace HashColon::Feline
{
	// mutex for ctime functions
	mutex ctime_mx;

	// defined as extern in ValueTypes.hpp
	Real _northPole = 90.0;
	Real _southPole = -90.0;

	TimePoint::TimePoint(string datetimeStr) { fromString(datetimeStr); }
	TimePoint::TimePoint(pair<string, string> timedef) { fromString(timedef.first, timedef.second); }
		
	//{ TimePoint functions
	void TimePoint::fromString(string datetimeStr, const string formatStr)
	{
		tm temp_tm = { 0 };
		stringstream ss(datetimeStr.c_str());
		ss >> get_time(&temp_tm, formatStr.c_str());

		// check if the datetimestr satisfies the format
		if (ss.fail() || !ss)
			throw out_of_range("datetime string out of range ( HashColon::Feline::Types::Timepoint::fromString() ).");
		else
		{
			time_t temptime_t;

			// unfortunately, mktime and localtime from ctime is not threadsafe.
			// therefore a lock should be provided.
			// refer to the following link for more information about tregedic behavior of mktime & localtime
			//https://stackoverflow.com/questions/16575029/localtime-not-thread-safe-but-okay-to-call-in-only-one-thread
			{
				lock_guard<mutex> lock_mx(ctime_mx);
				temptime_t = mktime(&temp_tm);
			}
			auto temp_tp = chrono::system_clock::from_time_t(temptime_t);
			(*this) = temp_tp;
		}
	}

	TimePoint& TimePoint::operator=(string datetimeStr) { fromString(datetimeStr); return (*this); }
	TimePoint& TimePoint::operator=(pair<string, string> timedef) { fromString(timedef.first, timedef.second); return (*this); }

	string TimePoint::toString(const string formatStr) const
	{
		time_t this_C = chrono::system_clock::to_time_t(*this);
		stringstream ss;

		// unfortunately, mktime and localtime from ctime is not threadsafe.
		// therefore a lock should be provided.
		// refer to the following link for more information about tregedic behavior of mktime & localtime
		//https://stackoverflow.com/questions/16575029/localtime-not-thread-safe-but-okay-to-call-in-only-one-thread
		{
			lock_guard<mutex> lock_mx(ctime_mx);
			ss << put_time(localtime(&this_C), formatStr.c_str());
		}
		return ss.str();
	}
}

// Position
namespace HashColon::Feline
{
	Real Position::DistanceTo(Position toPoint) const
	{
		#ifdef Feline_DISTANCE_USING_GRANDROUTE
			return DistanceTo_usingGrandRoute(toPoint);
		#else
			return DistanceTo_usingCartesianDistance(toPoint);
		#endif
	}

	Position Position::MoveTo(Real distanceMeter, Degree a) const
	{
		#ifdef Feline_DISTANCE_USING_GRANDROUTE
			return MoveTo_usingGrandRoute(a, distanceMeter);
		#else
			return MoveTo_usingCartesianDistance(a, distanceMeter);
		#endif
	}

	Degree Position::AngleTo(Position toPoint) const
	{
		#ifdef Feline_DISTANCE_USING_GRANDROUTE
			return AngleTo_usingGrandRoute(toPoint);
		#else
			return AngleTo_usingCartesianDistance(toPoint);
		#endif
	}

	Real Position::CrossTrackDistanceTo(Position path_s, Position path_e) const
	{
		#ifdef Feline_DISTANCE_USING_GRANDROUTE
			return CrossTrackDistanceTo_usingGrandRoute(path_s, path_e);
		#else
			return CrossTrackDistanceTo_usingCartesianDistance(path_s, path_e);
		#endif
	}

	Real Position::DistanceTo_usingCartesianDistance(Position toPoint) const
	{
		return Distance_CartesianDistance((*this), toPoint);
	}

	Real Position::DistanceTo_usingGrandRoute(Position toPoint) const
	{
		throw NotImplementedException;
	}

	Position Position::MoveTo_usingCartesianDistance(Real distanceMeter, Degree a) const
	{
		return MovePoint_CartesianDistance((*this), distanceMeter, a);
	}

	Position Position::MoveTo_usingGrandRoute(Real distanceMeter, Degree a) const
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

	Real Position::CrossTrackDistanceTo_usingCartesianDistance(Position path_s, Position path_e) const
	{
		return CrossTrackDistance_CartesianDistance((*this), path_s, path_e);
	}

	Real Position::CrossTrackDistanceTo_usingGrandRoute(Position path_s, Position path_e) const
	{
		throw NotImplementedException;
	}

	bool operator==(const Position& lhs, const Position& rhs)
	{
		return lhs.dat[0] == rhs.dat[0] && lhs.dat[1] == rhs.dat[1];
	}
	bool operator!=(const Position& lhs, const Position& rhs)
	{
		return !(lhs == rhs);
	}
}

// Velocity
namespace HashColon::Feline
{	
	bool operator==(const Velocity& lhs, const Velocity& rhs)
	{
		return lhs.dat[0] == rhs.dat[0] && lhs.dat[1] == rhs.dat[1];
	}
	bool operator!=(const Velocity& lhs, const Velocity& rhs)
	{
		return !(lhs == rhs);
	}
}

// XTD
namespace HashColon::Feline
{
	bool operator==(const XTD& lhs, const XTD& rhs)
	{
		return lhs.dat[0] == rhs.dat[0] && lhs.dat[1] == rhs.dat[1];
	}
	bool operator!=(const XTD& lhs, const XTD& rhs)
	{
		return !(lhs == rhs);
	}
}

// XYT
namespace HashColon::Feline
{
	Real XYT::DistanceTo(Position toPoint) const { return Pos.DistanceTo(toPoint); }
	Degree XYT::AngleTo(Position toPoint) const { return Pos.AngleTo(toPoint); }
	Real XYT::SpeedTo(XYT xyt) const { return Speed(Pos, TP, xyt.Pos, xyt.TP); }
	Velocity XYT::VelocityTo(XYT xyt) const { return VelocityBtwn(Pos, TP, xyt.Pos, xyt.TP); }

	bool operator==(const XYT& lhs, const XYT& rhs)
	{
		return lhs.Pos == rhs.Pos && lhs.TP == rhs.TP;
	}
	bool operator!=(const XYT& lhs, const XYT& rhs)
	{
		return !(lhs == rhs);
	}
}
	
// XYVVaT
namespace HashColon::Feline
{
	Real XYVVaT::DistanceTo(Position toPoint) const { return Pos.DistanceTo(toPoint); }
	Degree XYVVaT::AngleTo(Position toPoint) const { return Pos.AngleTo(toPoint); }
	Real XYVVaT::SpeedTo(XYT xyt) const { return Speed(Pos, TP, xyt.Pos, xyt.TP); }
	Velocity XYVVaT::VelocityTo(XYT xyt) const { return VelocityBtwn(Pos, TP, xyt.Pos, xyt.TP); }

	bool operator==(const XYVVaT& lhs, const XYVVaT& rhs)
	{
		return lhs.Pos == rhs.Pos && lhs.TP == rhs.TP && lhs.Vel == rhs.Vel;
	}
	bool operator!=(const XYVVaT& lhs, const XYVVaT& rhs)
	{
		return !(lhs == rhs);
	}
}

// XYXtd
namespace HashColon::Feline
{
	Real XYXtd::DistanceTo(Position toPoint) const { return Pos.DistanceTo(toPoint); }
	Degree XYXtd::AngleTo(Position toPoint) const { return Pos.AngleTo(toPoint); }
	
	bool operator==(const XYXtd& lhs, const XYXtd& rhs)
	{
		return lhs.Pos == rhs.Pos && lhs.Xtd == rhs.Xtd;
	}
	bool operator!=(const XYXtd& lhs, const XYXtd& rhs)
	{
		return !(lhs == rhs);
	}
}

// XYVVaXtdT
namespace HashColon::Feline
{
	Real XYVVaXtdT::DistanceTo(Position toPoint) const { return Pos.DistanceTo(toPoint); }
	Degree XYVVaXtdT::AngleTo(Position toPoint) const { return Pos.AngleTo(toPoint); }
	Real XYVVaXtdT::SpeedTo(XYT xyt) const { return Speed(Pos, TP, xyt.Pos, xyt.TP); }
	Velocity XYVVaXtdT::VelocityTo(XYT xyt) const { return VelocityBtwn(Pos, TP, xyt.Pos, xyt.TP); }

	bool operator==(const XYVVaXtdT& lhs, const XYVVaXtdT& rhs)
	{
		return lhs.Pos == rhs.Pos
			&& lhs.Vel == rhs.Vel
			&& lhs.Xtd == rhs.Xtd
			&& lhs.TP == rhs.TP;
	}
	bool operator!=(const XYVVaXtdT& lhs, const XYVVaXtdT& rhs)
	{
		return !(lhs == rhs);
	}
}

// XYList
namespace HashColon::Feline
{
	Real XYList::GetDistance(size_t sIndex, size_t eIndex) const
	{
		// check validity					
		if (eIndex >= this->size())
			eIndex = this->size() - 1;
		assert(sIndex < this->size());
		assert(sIndex <= eIndex);

		Real re = 0;
		for (size_t i = sIndex; i < eIndex; i++)
			re += at(i).DistanceTo(at(i + 1));

		return re;
	}

	XYList XYList::GetNormalizedXYList(size_t sizeN) const
	{
		XYList re;
		re.clear();  re.resize(sizeN + 1);

		// Step 1. Parameterization of the route
		vector<Real> uVec = GetParameterized();
		Real stepu = GetDistance() / ((Real)sizeN);

		// Step 2. Point sample from paraemeter u = k * stepL;
		size_t ptcnt1 = 0;
		size_t ptcnt2 = 0;
		size_t ptcnt2_old = 0;
		Real u = 0.0;
		for (size_t i = 0; i < sizeN; u += stepu, i++)
		{
			// uVec[ptcnt] < u < uVec[ptcnt + 1]
			ptcnt2_old = ptcnt2;
			while (u >= uVec[ptcnt2] && ptcnt2 < uVec.size() - 1)
			{
				ptcnt2++;
			}
			ptcnt1 = ptcnt2_old != ptcnt2 ? ptcnt2_old : ptcnt1;

			{
				using namespace HashColon::Vec2D;
				/*               uVec[ptcnt2] - u                               u - uVec[ptcnt1]
				pt[ptcnt1] * ----------------------------- + pt[ptcnt2] * -----------------------------
				uVec[ptcnt2] - uVec[ptcnt1]                   uVec[ptcnt2] - uVec[ptcnt1]
				*/
				re[i].dat = plus<Real>(
					multiply<Real>((uVec[ptcnt2] - u) / (uVec[ptcnt2] - uVec[ptcnt1]), this->at(ptcnt1).dat),
					multiply<Real>((u - uVec[ptcnt1]) / (uVec[ptcnt2] - uVec[ptcnt1]), this->at(ptcnt2).dat));
			}
		}

		re[sizeN] = this->back();

		return re;
	}

	vector<Real> XYList::GetParameterized() const
	{
		vector<Real> uVec;
		uVec.clear(); uVec.resize(size());

		for (size_t i = 0; i < size(); i++)
			uVec[i] = GetDistance(0, i);

		return uVec;
	}

	XYList XYList::GetReversed() const
	{
		XYList re;
		//re.reserve(size());
		re.resize(size());
		reverse_copy(begin(), end(), re.begin());
		return re;
	}
}

// XYTList
namespace HashColon::Feline
{
	Real XYTList::GetDistance(size_t sIndex, size_t eIndex) const
	{
		// check validity					
		if (eIndex >= this->size())
			eIndex = this->size() - 1;
		assert(sIndex < this->size());
		assert(sIndex <= eIndex);

		Real re = 0;
		for (size_t i = sIndex; i < eIndex; i++)
			re += at(i).DistanceTo(at(i + 1));

		return re;
	}

	Duration XYTList::GetDuration(size_t sIndex, size_t eIndex) const
	{
		// check validity					
		if (eIndex >= this->size())
			eIndex = this->size() - 1;
		assert(sIndex < this->size());
		assert(sIndex <= eIndex);

		TimePoint sTP = at(sIndex);
		TimePoint eTP = at(eIndex);

		Duration re = eTP - sTP;
		return re;
	}

	XYList XYTList::ToXYList() const
	{
		XYList re;
		re.clear();
		for (const XYT& it : (*this))
		{
			re.push_back(it);
		}
		return re;
	}
	XYTList XYTList::GetReversed() const
	{
		XYTList re;
		re.reserve(size());
		reverse_copy(begin(), end(), re.begin());
		return re;
	}
}

// XYVVaTList
namespace HashColon::Feline
{
	Real XYVVaTList::GetDistance(size_t sIndex, size_t eIndex) const
	{
		// check validity					
		if (eIndex >= this->size())
			eIndex = this->size() - 1;
		assert(sIndex < this->size());
		assert(sIndex <= eIndex);

		Real re = 0;
		for (size_t i = sIndex; i < eIndex; i++)
		{
			re += at(i).DistanceTo(at(i + 1));
		}
		return re;
	}

	Duration XYVVaTList::GetDuration(size_t sIndex, size_t eIndex) const
	{
		// check validity					
		if (eIndex >= this->size())
			eIndex = this->size() - 1;
		assert(sIndex < this->size());
		assert(sIndex <= eIndex);

		TimePoint sTP = at(sIndex);
		TimePoint eTP = at(eIndex);

		Duration re = eTP - sTP;
		return re;
	}

	XYList XYVVaTList::ToXYList() const
	{
		XYList re;
		re.clear();
		for (const XYVVaT& it : *(this))
		{
			re.push_back(it);
		}
		return re;
	}

	XYTList XYVVaTList::ToXYTList() const
	{
		XYTList re;
		re.clear();
		for (const XYVVaT& it : *(this))
		{			
			re.push_back(it);
		}
		return re;
	}

	XYVVaTList XYVVaTList::GetReversed() const
	{
		XYVVaTList re;
		re.reserve(size());
		reverse_copy(begin(), end(), re.begin());
		return re;
	}
}

// XYXtdList
namespace HashColon::Feline
{
	Real XYXtdList::GetDistance(size_t sIndex, size_t eIndex) const
	{
		// check validity					
		if (eIndex >= this->size())
			eIndex = this->size() - 1;
		assert(sIndex < this->size());
		assert(sIndex <= eIndex);

		Real re = 0;
		for (size_t i = sIndex; i < eIndex; i++)
			re += at(i).DistanceTo(at(i + 1));

		return re;
	}

	XYList XYXtdList::ToXYList() const
	{
		XYList re;
		re.clear();
		for (const XYXtd& it : (*this))
		{
			re.push_back(it.Pos);
		}
		return re;
	}

	XYXtdList XYXtdList::GetNormalizedXYXtdList(size_t sizeN) const
	{
		XYXtdList re;
		re.clear();  re.resize(sizeN + 1);

		// Step 1. Parameterization of the route
		vector<Real> uVec = ToXYList().GetParameterized();
		Real stepu = GetDistance() / ((Real)sizeN);

		// Step 2. Point sample from paraemeter u = k * stepL;
		size_t ptcnt1 = 0;
		size_t ptcnt2 = 0;
		size_t ptcnt2_old = 0;
		Real u = 0.0;
		for (size_t i = 0; i < sizeN; u += stepu, i++)
		{
			// uVec[ptcnt] < u < uVec[ptcnt + 1]
			ptcnt2_old = ptcnt2;
			while (u >= uVec[ptcnt2] && ptcnt2 < uVec.size() - 1)
			{
				ptcnt2++;
			}
			ptcnt1 = ptcnt2_old != ptcnt2 ? ptcnt2_old : ptcnt1;

			{
				using namespace HashColon::Vec2D;
				/*               uVec[ptcnt2] - u                               u - uVec[ptcnt1]
				pt[ptcnt1] * ----------------------------- + pt[ptcnt2] * -----------------------------
				uVec[ptcnt2] - uVec[ptcnt1]                   uVec[ptcnt2] - uVec[ptcnt1]
				*/
				assert(uVec.size() > ptcnt1);
				assert(uVec.size() > ptcnt2);
				assert(size() > ptcnt1);
				assert(size() > ptcnt2);

				array<Real, 2> tmppos = plus<Real>(
					multiply<Real>((uVec[ptcnt2] - u) / (uVec[ptcnt2] - uVec[ptcnt1]), this->at(ptcnt1).Pos.dat),
					multiply<Real>((u - uVec[ptcnt1]) / (uVec[ptcnt2] - uVec[ptcnt1]), this->at(ptcnt2).Pos.dat));
				Position temp_pos{ tmppos[0], tmppos[1] };
				XTD temp_xtd = this->at(ptcnt1);
				re[i] = XYXtd(temp_pos, temp_xtd);
			}
		}
		re[sizeN] = this->back();
		return re;
	}

	XYXtdList XYXtdList::GetReversed() const
	{
		XYXtdList re;		
		re.resize(size());
		reverse_copy(begin(), end(), re.begin());
		return re;
	}
}

// XYVVaXtdTList
namespace HashColon::Feline
{
	Real XYVVaXtdTList::GetDistance(size_t sIndex, size_t eIndex) const
	{
		// check validity					
		if (eIndex >= this->size())
			eIndex = this->size() - 1;
		assert(sIndex < this->size());
		assert(sIndex <= eIndex);

		Real re = 0;
		for (size_t i = sIndex; i < eIndex; i++)
		{
			re += at(i).DistanceTo(at(i + 1));
		}
		return re;
	}

	Duration XYVVaXtdTList::GetDuration(size_t sIndex, size_t eIndex) const
	{
		// check validity					
		if (eIndex >= this->size())
			eIndex = this->size() - 1;
		assert(sIndex < this->size());
		assert(sIndex <= eIndex);

		TimePoint sTP = at(sIndex);
		TimePoint eTP = at(eIndex);

		Duration re = eTP - sTP;
		return re;
	}

	XYVVaXtdTList XYVVaXtdTList::GetReversed() const
	{
		XYVVaXtdTList re;
		re.reserve(size());
		reverse_copy(begin(), end(), re.begin());
		return re;
	}

	XYList XYVVaXtdTList::ToXYList() const
	{
		XYList re;
		re.clear(); re.reserve(size());
		for (const XYVVaT& it : *(this))
		{
			re.push_back(it);
		}
		return re;
	}

	XYTList XYVVaXtdTList::ToXYTList() const
	{
		XYTList re;	
		re.clear(); re.reserve(size());
		for (const XYVVaT& it : *(this))
		{
			re.push_back(it);
		}
		return re;
	}

	XYXtdList XYVVaXtdTList::ToXYXtdList() const
	{
		XYXtdList re;
		re.clear(); re.reserve(size());
		for (const XYVVaXtdT& it : (*this))
		{
			re.push_back(it);
		}
		return re;
	}
}

// Basic Computing functions
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
			using namespace HashColon::Vec2D;
			array<Real, 2> temp;
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
		using namespace HashColon::Vec2D;
		array<Real, 2> temp;
		temp = minus<Real>(B.dat, A.dat);
		temp[0] *= _LonUnitDist;
		temp[1] *= _LatUnitDist;
		Real dist = abs<Real>(temp);

		return (
			(temp[0] >= 0) ?
			acos(temp[1] / dist) :
			2 * Constant::PI - acos(temp[1] / dist)
			) * 180 / Constant::PI;
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

		re.longitude += d * sin(alpha * Constant::PI / 180) / _LonUnitDist;
		re.latitude += d * cos(alpha * Constant::PI / 180) / _LatUnitDist;

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
		using namespace HashColon::Vec2D;
		array<Real, 2> _P{ { _LonUnitDist * P.longitude, _LatUnitDist * P.latitude} };
		array<Real, 2> _S{ { _LonUnitDist * path_S.longitude, _LatUnitDist * path_S.latitude} };
		array<Real, 2> _E{ { _LonUnitDist * path_E.longitude, _LatUnitDist * path_E.latitude} };
		return PointToLineDistance(_S, _E, _P);
	}

	Real CrossTrackDistance_GrandRoute(const Position& P, const Position& path_S, const Position& path_E)
	{
		throw NotImplementedException;
	}

	// Speed
	Real Speed_CartesianDistance(const Position& A, const TimePoint& AT, const Position& B, const TimePoint& BT)
	{
		return Distance_CartesianDistance(A, B) / (Real)(AT - BT).count();
	}
	Real Speed_GrandRoute(const Position& A, const TimePoint& AT, const Position& B, const TimePoint& BT)
	{
		return Distance_GrandRoute(A, B) / (Real)(AT - BT).count();
	}
	Real Speed(const Position& A, const TimePoint& AT, const Position& B, const TimePoint& BT)
	{
	#ifdef Feline_DISTANCE_USING_GRANDROUTE
		return Speed_GrandRoute(A, AT, B, BT);
	#else
		return Speed_CartesianDistance(A, AT, B, BT);
	#endif
	}

	// Velocity
	Velocity VelocityBtwn_CartesianDistance(const Position& A, const TimePoint& AT, const Position& B, const TimePoint& BT)
	{
		return { Speed_CartesianDistance(A, AT, B, BT), Angle_CartesianDistance(A, B) };
	}
	Velocity VelocityBtwn_GrandRoute(const Position& A, const TimePoint& AT, const Position& B, const TimePoint& BT)
	{
		return { Speed_GrandRoute(A, AT, B, BT), Angle_GrandRoute(A, B) };
	}
	Velocity VelocityBtwn(const Position& A, const TimePoint& AT, const Position& B, const TimePoint& BT)
	{
		return { Speed(A, AT, B, BT), Angle(A, B) };
	}
}
