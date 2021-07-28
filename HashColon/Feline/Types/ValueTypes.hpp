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

#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif


#include <HashColon/Feline/Feline_config.h>

/* c++11 features should be supported
 __cplusplus > 197711L || _MSC_VER >= 1600*/

#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include <Eigen/Eigen>

#include <HashColon/Core/Real.hpp>

namespace HashColon::Feline
{

	// longitude definition
	using LongitudeType = HashColon::Degree;

	// latitude definition
	extern HashColon::Real _northPole;
	extern HashColon::Real _southPole;
	using LatitudeType = HashColon::BoundedReal<_southPole, _northPole>;



	// ship id definition
	using ShipIDKey = unsigned int;

	// TimeInterval definition as real
	using Duration = std::chrono::system_clock::duration;

	// datetime definition
	class TimePoint : public std::chrono::system_clock::time_point
	{
	public:
		constexpr TimePoint()
			: std::chrono::system_clock::time_point() {};

		constexpr explicit TimePoint(const Duration& d)
			: std::chrono::system_clock::time_point(d) {};

		template <class Duration2>
		constexpr TimePoint(const time_point<std::chrono::system_clock, Duration2>& t)
			: std::chrono::system_clock::time_point(t) {};

	public:
		void fromString(const std::string formatStr, std::string datetimeStr);
		std::string toString(const std::string formatStr) const;		
	};


	// hidden classes for Position, Velocity, Xtd
	namespace _hidden
	{
		struct _Position
		{
			union {
				struct
				{
					HashColon::Real longitude;
					HashColon::Real latitude;
				};
				std::array<HashColon::Real, 2> dat;
			};						
		};
		bool operator==(const _Position& lhs, const _Position& rhs);
		bool operator!=(const _Position& lhs, const _Position& rhs);

		struct _Velocity
		{
			union {
				struct
				{
					HashColon::Real speed;
					HashColon::Real angle;
				};
				std::array<HashColon::Real, 2> dat;
			};		
		};
		bool operator==(const _Velocity& lhs, const _Velocity& rhs);
		bool operator!=(const _Velocity& lhs, const _Velocity& rhs);

		struct _XTD
		{
			union {
				struct
				{
					HashColon::Real xtdPortside;
					HashColon::Real xtdStarboard;
				};
				std::array<HashColon::Real, 2> dat;
			};		
		};
		bool operator==(const _XTD& lhs, const _XTD& rhs);
		bool operator!=(const _XTD& lhs, const _XTD& rhs);
	}

	// position definition			
	struct Position : public _hidden::_Position
	{
		inline Position(const _hidden::_Position& p)
		{
			longitude = p.longitude;
			latitude = p.latitude;
		}

		inline Position()
		{
			longitude = latitude = 0.0;
		}

		inline Position(HashColon::Real _lon, HashColon::Real _lat)
		{
			longitude = _lon;
			latitude = _lat;
		}

		inline HashColon::Real DistanceTo(Position toPoint) const
		{
#ifdef Feline_DISTANCE_USING_GRANDROUTE
			return DistanceTo_usingGrandRoute(toPoint);
#else
			return DistanceTo_usingCartesianDistance(toPoint);
#endif
		}

		inline Position MoveTo(HashColon::Degree a, HashColon::Real distanceMeter) const
		{
#ifdef Feline_DISTANCE_USING_GRANDROUTE
			return MoveTo_usingGrandRoute(a, distanceMeter);
#else
			return MoveTo_usingCartesianDistance(a, distanceMeter);
#endif
		}

		inline HashColon::Degree AngleTo(Position toPoint) const
		{
#ifdef Feline_DISTANCE_USING_GRANDROUTE
			return AngleTo_usingGrandRoute(toPoint);
#else
			return AngleTo_usingCartesianDistance(toPoint);
#endif
		}

		inline HashColon::Real CrossTrackDistanceTo(Position path_s, Position path_e)
		{
#ifdef Feline_DISTANCE_USING_GRANDROUTE
			return CrossTrackDistanceTo_usingGrandRoute(path_s, path_e);
#else
			return CrossTrackDistanceTo_usingCartesianDistance(path_s, path_e);
#endif
		}

		HashColon::Real DistanceTo_usingCartesianDistance(Position toPoint) const;
		HashColon::Real DistanceTo_usingGrandRoute(Position toPoint) const;

		Position MoveTo_usingCartesianDistance(HashColon::Degree a, HashColon::Real distanceMeter) const;
		Position MoveTo_usingGrandRoute(HashColon::Degree a, HashColon::Real distanceMeter) const;

		HashColon::Degree AngleTo_usingCartesianDistance(Position toPoint) const;
		HashColon::Degree AngleTo_usingGrandRoute(Position toPoint) const;

		HashColon::Real CrossTrackDistanceTo_usingGrandRoute(Position path_s, Position path_e);
		HashColon::Real CrossTrackDistanceTo_usingCartesianDistance(Position path_s, Position path_e);

		inline HashColon::Real& operator[](std::size_t i) { return dat[i]; }
	};

	// velocity definition
	struct Velocity : public _hidden::_Velocity
	{
		inline Velocity(const _hidden::_Velocity& v)
		{
			speed = v.speed;
			angle = v.angle;
		}

		inline Velocity()
		{
			speed = angle = 0.0;
		}

		inline Velocity(HashColon::Real _speed, HashColon::Real _angle)
		{
			speed = _speed;
			angle = _angle;
		}

		inline HashColon::Real& operator[](std::size_t i) { return dat[i]; }
	};

	struct XTD : public _hidden::_XTD
	{
		inline XTD(const _hidden::_XTD& xtd)
		{
			xtdPortside = xtd.xtdPortside;
			xtdStarboard = xtd.xtdStarboard;
		}

		inline XTD()
		{
			xtdPortside = xtdStarboard = 0;
		}
		inline XTD(HashColon::Real _xtdp, HashColon::Real _xtds)
		{
			xtdPortside = _xtdp; xtdStarboard = _xtds;
		}

		inline HashColon::Real& operator[](std::size_t i) { return dat[i]; }
	};


	constexpr HashColon::Real _LonUnitDist = 88.74 * 1000;  // 88.74km / degree
	constexpr HashColon::Real _LatUnitDist = 109.96 * 1000; // 109.96km / degree
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
}

#endif
