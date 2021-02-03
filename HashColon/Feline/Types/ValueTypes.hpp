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

#include <HashColon/Helper/Real.hpp>

namespace HashColon
{
	namespace Feline
	{
		namespace Types
		{			
			// longitude definition
			using LongitudeType = HashColon::Helper::Degree;

			// latitude definition
			extern HashColon::Helper::Real _northPole;
			extern HashColon::Helper::Real _southPole;
			using LatitudeType = HashColon::Helper::BoundedReal<_southPole, _northPole>;

			

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

				constexpr explicit TimePoint(const Duration &d)
					: std::chrono::system_clock::time_point(d) {};
				 
				template <class Duration2>
				constexpr TimePoint(const time_point<std::chrono::system_clock, Duration2> &t)
					: std::chrono::system_clock::time_point(t) {};
				
			public:
				void fromString(const std::string formatStr, std::string datetimeStr);
				std::string toString(const std::string formatStr) const;
			};


			// hidden classes for Position, Velocity
			namespace hidden
			{
				struct _Position
				{
					union {
						struct
						{
							HashColon::Helper::Real longitude;
							HashColon::Helper::Real latitude;
						};
						std::array<HashColon::Helper::Real, 2> dat;
					};
				};

				struct _Velocity
				{
					union {
						struct
						{
							HashColon::Helper::Real speed;
							HashColon::Helper::Real angle;
						};
						std::array<HashColon::Helper::Real, 2> dat;
					};
				};

			}

			// position definition			
			struct Position : public hidden::_Position
			{
				inline Position(const hidden::_Position &p)
				{
					longitude = p.longitude;
					latitude = p.latitude;
				}

				inline Position()
				{
					longitude = latitude = 0.0;
				}

				inline Position(HashColon::Helper::Real _lon, HashColon::Helper::Real _lat)
				{
					longitude = _lon;
					latitude = _lat;
				}

				inline HashColon::Helper::Real DistanceTo(Position toPoint) const
				{
#ifdef Feline_DISTANCE_USING_GRANDROUTE
					return DistanceTo_usingGrandRoute(toPoint);
#else
					return DistanceTo_usingCartesianDistance(toPoint);
#endif
				}

				inline Position MoveTo(HashColon::Helper::Degree a, HashColon::Helper::Real distanceMeter) const
				{
#ifdef Feline_DISTANCE_USING_GRANDROUTE
					return MoveTo_usingGrandRoute(a, distanceMeter);
#else
					return MoveTo_usingCartesianDistance(a, distanceMeter);
#endif
				}

				inline HashColon::Helper::Degree AngleTo(Position toPoint) const
				{
#ifdef Feline_DISTANCE_USING_GRANDROUTE
					return AngleTo_usingGrandRoute(toPoint);
#else
					return AngleTo_usingCartesianDistance(toPoint);
#endif
				}

				HashColon::Helper::Real DistanceTo_usingCartesianDistance(Position toPoint) const;
				HashColon::Helper::Real DistanceTo_usingGrandRoute(Position toPoint) const;

				Position MoveTo_usingCartesianDistance(HashColon::Helper::Degree a, HashColon::Helper::Real distanceMeter) const;
				Position MoveTo_usingGrandRoute(HashColon::Helper::Degree a, HashColon::Helper::Real distanceMeter) const;

				HashColon::Helper::Degree AngleTo_usingCartesianDistance(Position toPoint) const;
				HashColon::Helper::Degree AngleTo_usingGrandRoute(Position toPoint) const;

				inline HashColon::Helper::Real& operator[](std::size_t i) { return dat[i]; }
			};

			// velocity definition
			struct Velocity : public hidden::_Velocity
			{
				inline Velocity(const hidden::_Velocity &v)
				{
					speed = v.speed;
					angle = v.angle;
				}

				inline Velocity()
				{
					speed = angle = 0.0;
				}

				inline Velocity(HashColon::Helper::Real _speed, HashColon::Helper::Real _angle)
				{
					speed = _speed;
					angle = _angle;
				}

				inline HashColon::Helper::Real& operator[](std::size_t i) { return dat[i]; }
			};


			constexpr HashColon::Helper::Real _LonUnitDist = 88.74;  // 88.74km / degree
			constexpr HashColon::Helper::Real _LatUnitDist = 109.96; // 109.96km / degree
		}
	}
}

#endif
