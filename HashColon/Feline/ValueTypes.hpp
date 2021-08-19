// ***********************************************************************
// Assembly         : HashColon.Feline
// Author           : Wonchul Yoo
// Created          : 11-07-2017
//
// Last Modified By : Wonchul Yoo
// Last Modified On : 01-22-2018
// ***********************************************************************
// <copyright file="ValueTypes.hpp" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary>Defines basic types for Feline</summary>
// ***********************************************************************

#ifndef HASHCOLON_FELINE_TYPES_VALUETYPES_HPP
#define HASHCOLON_FELINE_TYPES_VALUETYPES_HPP

// std libraries
#include <array>
#include <chrono>
#include <string>
#include <utility>
#include <vector>
// HashColon libraries
#include <HashColon/Real.hpp>

// Base types & point types
namespace HashColon::Feline
{
	// longitude definition
	using LongitudeType = HashColon::Degree;

	// latitude definition
	extern HashColon::Real _northPole;
	extern HashColon::Real _southPole;
	using LatitudeType = HashColon::BoundedReal<_southPole, _northPole>;

	// unit distance near Korean peninsula
	constexpr HashColon::Real _LonUnitDist = 88.74 * 1000;  // 88.74km / degree
	constexpr HashColon::Real _LatUnitDist = 109.96 * 1000; // 109.96km / degree

	// ship id definition
	using ShipIDKey = unsigned int;

	// TimeInterval definition as real
	using Duration = std::chrono::system_clock::duration;

	// datetime definition
	class TimePoint : public std::chrono::system_clock::time_point
	{
	public:
		inline static const std::string defaultFormat = "yy-mm-dd HH:MM:SS";

		constexpr TimePoint()
			: std::chrono::system_clock::time_point() {};

		constexpr explicit TimePoint(const Duration& d)
			: std::chrono::system_clock::time_point(d) {};

		template <class Duration2>
		constexpr TimePoint(const time_point<std::chrono::system_clock, Duration2>& t)
			: std::chrono::system_clock::time_point(t) {};

		TimePoint(std::string datetimeStr);
		TimePoint(std::pair<std::string, std::string> timedef);

	public:		
		TimePoint& operator=(std::string datetimeStr);
		TimePoint& operator=(std::pair<std::string, std::string> timedef);
		void fromString(std::string datetimeStr, const std::string formatStr = defaultFormat);
		std::string toString(const std::string formatStr = defaultFormat) const;
		
	};

	// position definition			
	struct Position
	{
		union {
			struct
			{
				HashColon::Real longitude;
				HashColon::Real latitude;
			};
			std::array<HashColon::Real, 2> dat;
		};

		/*constexpr Position() : longitude(0), latitude(0) {};
		constexpr Position(const Position& p) : longitude(p.longitude), latitude(p.latitude) {};
		constexpr Position(const HashColon::Real lon, const HashColon::Real lat) : longitude(lon), latitude(lat) {};*/

		HashColon::Real& operator[](std::size_t i) { return dat[i]; }

		HashColon::Real DistanceTo(Position toPoint) const;
		Position MoveTo(HashColon::Real distanceMeter, HashColon::Degree a) const;
		HashColon::Degree AngleTo(Position toPoint) const;
		HashColon::Real CrossTrackDistanceTo(Position path_s, Position path_e) const;
		
		HashColon::Real DistanceTo_usingCartesianDistance(Position toPoint) const;
		HashColon::Real DistanceTo_usingGrandRoute(Position toPoint) const;
		Position MoveTo_usingCartesianDistance(HashColon::Real distanceMeter, HashColon::Degree a) const;
		Position MoveTo_usingGrandRoute(HashColon::Real distanceMeter, HashColon::Degree a) const;
		HashColon::Degree AngleTo_usingCartesianDistance(Position toPoint) const;
		HashColon::Degree AngleTo_usingGrandRoute(Position toPoint) const;
		HashColon::Real CrossTrackDistanceTo_usingGrandRoute(Position path_s, Position path_e) const;
		HashColon::Real CrossTrackDistanceTo_usingCartesianDistance(Position path_s, Position path_e) const;
	};
	bool operator==(const Position& lhs, const Position& rhs);
	bool operator!=(const Position& lhs, const Position& rhs);
	using XY = Position;

	// velocity definition
	struct Velocity
	{
		union {
			struct
			{
				HashColon::Real speed;
				HashColon::Real angle;
			};
			std::array<HashColon::Real, 2> dat;
		};

		/*constexpr Velocity() : speed(0), angle(0) {};
		constexpr Velocity(const Velocity& v) : speed(v.speed), angle(v.angle) {};
		constexpr Velocity(const HashColon::Real s, const HashColon::Real a) : speed(s), angle(a) {};*/
		
		HashColon::Real& operator[](std::size_t i) { return dat[i]; }
	};
	bool operator==(const Velocity& lhs, const Velocity& rhs);
	bool operator!=(const Velocity& lhs, const Velocity& rhs);
	using VVa = Velocity;

	struct XTD
	{		
		union {
			struct
			{
				HashColon::Real xtdPortside;
				HashColon::Real xtdStarboard;
			};
			std::array<HashColon::Real, 2> dat;
		};	
		
		/*constexpr XTD() : xtdPortside(0), xtdStarboard(0) {};
		constexpr XTD(const XTD& xtd) : xtdPortside(xtd.xtdPortside), xtdStarboard(xtd.xtdStarboard) {};
		constexpr XTD(const HashColon::Real xtdp, const HashColon::Real xtds) : xtdPortside(xtdp), xtdStarboard(xtds) {};*/
		
		HashColon::Real& operator[](std::size_t i) { return dat[i]; }
	};
	bool operator==(const XTD& lhs, const XTD& rhs);
	bool operator!=(const XTD& lhs, const XTD& rhs);

	struct XYT
	{
		Position Pos;
		TimePoint TP;
		constexpr XYT() : Pos(), TP() {};
		constexpr XYT(const XYT& xyt) : Pos(xyt.Pos), TP(xyt.TP) {};
		constexpr XYT(Position pos, TimePoint tp) : Pos(pos), TP(tp) {};
		constexpr XYT(const HashColon::Real x, const HashColon::Real y, TimePoint tp)
			: Pos({ x,y }), TP(tp) 
		{};
		
		HashColon::Real& operator[](std::size_t i) { return Pos.dat[i]; };
		
		HashColon::Real DistanceTo(Position toPoint) const;
		HashColon::Degree AngleTo(Position toPoint) const;
		HashColon::Real SpeedTo(XYT xyt) const;
		Velocity VelocityTo(XYT xyt) const;
		operator Position() const { return Pos; };
		operator TimePoint() const { return TP; };
	};
	bool operator==(const XYT& lhs, const XYT& rhs);
	bool operator!=(const XYT& lhs, const XYT& rhs);

	// V, Va are velocity of leg before XY
	// ex) wp2 -> leg(2,3) -> wp3 :
	//     for wp3, XYTVVa contains (X3, Y3, T3, V(leg(2,3)), Va(leg(2,3)))
	struct XYVVaT
	{
		union
		{
			struct
			{
				Position Pos;
				Velocity Vel;
			};
			std::array<HashColon::Real, 4> dat;
		};
		TimePoint TP;

		constexpr XYVVaT() : Pos(), Vel(), TP() {};
		constexpr XYVVaT(const XYVVaT& xyvvat) : Pos(xyvvat.Pos), Vel(xyvvat.Vel), TP(xyvvat.TP) {};
		constexpr XYVVaT(XY xy, VVa vva, TimePoint tp) : Pos(xy), Vel(vva), TP(tp) {};
		constexpr XYVVaT(
			HashColon::Real x, HashColon::Real y,
			HashColon::Real speed, HashColon::Real angle,
			TimePoint tp)
			: Pos({ x, y }), Vel({ speed, angle }), TP(tp) 
		{};
		
		HashColon::Real& operator[](std::size_t i) { return dat[i]; };

		HashColon::Real DistanceTo(Position toPoint) const;
		HashColon::Degree AngleTo(Position toPoint) const;
		HashColon::Real SpeedTo(XYT xyt) const;
		Velocity VelocityTo(XYT xyt) const;
		
		operator Position() const { return Pos; };
		operator Velocity() const { return Vel; };
		operator TimePoint() const { return TP; };
		operator XYT() const { return XYT(Pos, TP); };
	};
	bool operator==(const XYVVaT& lhs, const XYVVaT& rhs);
	bool operator!=(const XYVVaT& lhs, const XYVVaT& rhs);
		
	struct XYXtd
	{
		union
		{
			struct
			{
				Position Pos;
				XTD Xtd;
			};
			std::array<HashColon::Real, 4> dat;
		};

		constexpr XYXtd() : Pos(), Xtd() {};
		constexpr XYXtd(const XYXtd& xyxtd) : Pos(xyxtd.Pos), Xtd(xyxtd.Xtd) {};
		constexpr XYXtd(Position pos, XTD xtd) : Pos(pos), Xtd(xtd) {};
		constexpr XYXtd(
			HashColon::Real x, HashColon::Real y,
			HashColon::Real xtdp, HashColon::Real xtds)
			: Pos({ x, y }), Xtd({ xtdp, xtds })
		{};

		HashColon::Real& operator[](std::size_t i) { return dat[i]; };

		HashColon::Real DistanceTo(Position toPoint) const;
		HashColon::Degree AngleTo(Position toPoint) const;

		operator Position() const { return Pos; };
		operator XTD() const { return Xtd; };
	};
	bool operator==(const XYXtd& lhs, const XYXtd& rhs);
	bool operator!=(const XYXtd& lhs, const XYXtd& rhs);

	struct XYVVaXtdT
	{
		union
		{
			struct
			{
				Position Pos;
				Velocity Vel;
				XTD Xtd;
			};
			std::array<HashColon::Real, 6> dat;
		};
		TimePoint TP;
		
		constexpr XYVVaXtdT() : Pos(), Vel(), Xtd(), TP() {};
		constexpr XYVVaXtdT(const XYVVaXtdT& xyvvaxtdt)
			: Pos(xyvvaxtdt.Pos), Vel(xyvvaxtdt.Vel), Xtd(xyvvaxtdt.Xtd), TP(xyvvaxtdt.TP)
		{};
		constexpr XYVVaXtdT(XY xy, VVa vva, XTD xtd, TimePoint tp)
			: Pos(xy), Vel(vva), Xtd(xtd), TP(tp)
		{};
		constexpr XYVVaXtdT(
			HashColon::Real x, HashColon::Real y,
			HashColon::Real speed, HashColon::Real angle,
			HashColon::Real xtdp, HashColon::Real xtds,
			TimePoint tp)
			: Pos({ x, y }), Vel({ speed, angle }), Xtd({ xtdp, xtds }), TP(tp)
		{};

		HashColon::Real DistanceTo(Position toPoint) const;
		HashColon::Degree AngleTo(Position toPoint) const;
		HashColon::Real SpeedTo(XYT xyt) const;
		Velocity VelocityTo(XYT xyt) const;

		operator Position() const { return Pos; };
		operator Velocity() const { return Vel; };
		operator XTD() const { return Xtd; };
		operator TimePoint() const { return TP; };
		operator XYT() const { return XYT(Pos, TP); };
		operator XYVVaT() const { return XYVVaT(Pos, Vel, TP); };
		operator XYXtd() const { return XYXtd(Pos, Xtd); };
	};
	bool operator==(const XYVVaXtdT& lhs, const XYVVaXtdT& rhs);
	bool operator!=(const XYVVaXtdT& lhs, const XYVVaXtdT& rhs);
}

// list types
namespace HashColon::Feline
{
	class XYList : public std::vector<XY>
	{
	public:
		HashColon::Real GetDistance(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		XYList GetNormalizedXYList(size_t sizeN) const;
		std::vector<HashColon::Real> GetParameterized() const;
		XYList GetReversed() const;
	};

	class XYXtdList : public std::vector<XYXtd>
	{
	public:
		HashColon::Real GetDistance(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		XYXtdList GetNormalizedXYXtdList(size_t sizeN) const;
		XYXtdList GetReversed() const;
		XYList ToXYList() const;

		operator XYList() const { return ToXYList(); };
	};

	class XYTList : public std::vector<XYT>
	{
	public:
		HashColon::Real GetDistance(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		Duration GetDuration(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		XYTList GetReversed() const;
		XYList ToXYList() const;

		operator XYList() const { return ToXYList(); };
	};

	class XYVVaTList : public std::vector<XYVVaT>
	{
	public:
		HashColon::Real GetDistance(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		Duration GetDuration(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		XYVVaTList GetReversed() const;
		XYList ToXYList() const;
		XYTList ToXYTList() const;

		operator XYList() const { return ToXYList(); };
		operator XYTList() const { return ToXYTList(); };
	};

	class XYVVaXtdTList : public std::vector<XYVVaXtdT>
	{
	public:
		HashColon::Real GetDistance(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		Duration GetDuration(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		XYVVaXtdTList GetReversed() const;
		XYList ToXYList() const;
		XYXtdList ToXYXtdList() const;
		XYTList ToXYTList() const;
		XYVVaTList ToXYVVaTList() const;

		operator XYList() const { return ToXYList(); };
		operator XYXtdList() const { return ToXYXtdList(); };
		operator XYTList() const { return ToXYTList(); };
		operator XYVVaTList() const { return ToXYVVaTList(); };
	};
}

// list with vessel data
namespace HashColon::Feline
{
	struct StaticType
	{
		ShipIDKey imo;
		ShipIDKey mmsi;		
		struct {
			HashColon::Real L;
			HashColon::Real B;
			HashColon::Real T;
		} Dim;
	};

	template <typename TList = XYVVaTList>
	struct AisTrajectory
	{
		StaticType staticInfo;
		TList trajectory;
	};
}

// Basic Computing functions
namespace HashColon::Feline
{
	// Distance
	HashColon::Real Distance_CartesianDistance(const Position& A, const Position& B);
	HashColon::Real Distance_GrandRoute(const Position& A, const Position& B);
	HashColon::Real Distance(const Position& A, const Position& B);
		
	// Angle(angle btwn north)
	HashColon::Degree Angle_GrandRoute(const Position& A, const Position& B);
	HashColon::Degree Angle_CartesianDistance(const Position& A, const Position& B);
	HashColon::Degree Angle(const Position& A, const Position& B);
	
	// Angle (angle APB)
	HashColon::Degree Angle_GrandRoute(const Position& A, const Position& B, const Position& P);
	HashColon::Degree Angle_CartesianDistance(const Position& A, const Position& B, const Position& P);
	HashColon::Degree Angle(const Position& A, const Position& B, const Position& P);
	
	// Move point
	Position MovePoint_GrandRoute(const Position& A, const HashColon::Real& d, const HashColon::Degree& alpha);
	Position MovePoint_CartesianDistance(const Position& A, const HashColon::Real& d, const HashColon::Degree& alpha);
	Position MovePoint(Position A, HashColon::Real d, HashColon::Degree alpha);
	
	// Cross track distance
	HashColon::Real CrossTrackDistance_GrandRoute(const Position& P, const Position& path_S, const Position& path_E);
	HashColon::Real CrossTrackDistance_CartesianDistance(const Position& P, const Position& path_S, const Position& path_E);
	HashColon::Real CrossTrackDistance(const Position& P, const Position& path_S, const Position& path_E);	

	// Speed
	HashColon::Real Speed_CartesianDistance(const Position& A, const TimePoint& AT, const Position& B, const TimePoint& BT);
	HashColon::Real Speed_GrandRoute(const Position& A, const TimePoint& AT, const Position& B, const TimePoint& BT);
	HashColon::Real Speed(const Position& A, const TimePoint& AT, const Position& B, const TimePoint& BT);

	// Velocity
	Velocity VelocityBtwn_CartesianDistance(const Position& A, const TimePoint& AT, const Position& B, const TimePoint& BT);
	Velocity VelocityBtwn_GrandRoute(const Position& A, const TimePoint& AT, const Position& B, const TimePoint& BT);
	Velocity VelocityBtwn(const Position& A, const TimePoint& AT, const Position& B, const TimePoint& BT);
}

#endif
