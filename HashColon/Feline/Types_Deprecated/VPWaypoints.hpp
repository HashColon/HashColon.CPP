#ifndef HASHCOLON_FELINE_TYPES_VPWAYPOINTS_HPP
#define HASHCOLON_FELINE_TYPES_VPWAYPOINTS_HPP

#include <HashColon/Feline/Feline_config.h>
#include <memory>
#include <HashColon/Feline/GeoValues.hpp>
#include <HashColon/Feline/Types/VPBase.hpp>

namespace HashColon
{
	namespace Feline
	{
		namespace Types
		{
			/// <summary>
			/// XTD defintion
			/// </summary>
			struct XTD
			{
				union
				{
					struct
					{
						Real starboard;
						Real portside;
					};
					Real xtd[2];
				};
			};

			/// <summary>
			/// Enum LegGeometryTypeEnum
			/// </summary>
			enum LegGeometryTypeEnum
			{
				Loxodrome, Orthodrome
			};


			/// <summary>
			/// Leg defines the properties btwn waypoints
			/// </summary>s			
			class Leg
			{
			public:
				/// <summary>
				/// The XTD
				/// </summary>				
				XTD xtd;

				/// <summary>
				/// The safety contour
				/// </summary>
				Real safetyContour;

				/// <summary>
				/// The safety depth
				/// </summary>
				Real safetyDepth;

				/// <summary>
				/// The estimated speed
				/// </summary>
				Real estimatedSpeed;

				/// <summary>
				/// The engine power for the leg. (Not the maximum engine power)
				/// </summary>
				Real enginePower;

				/// <summary>
				/// The geometry type
				/// </summary>
				LegGeometryTypeEnum geometryType;

				/// <summary>
				/// The leg note
				/// </summary>
				Notes legNote;

				/// <summary>
				/// The starting waypoint of the leg
				/// </summary>				
				std::weak_ptr<Waypoint> waypoint;

				// pointer type definition
				using Ptr = std::shared_ptr<Leg>;
			};

			/// <summary>
			/// Defines a waypoint.
			/// </summary>
			/// <seealso cref="ComponentBase" />
			class Waypoint : public NamedComponentBase, public std::enable_shared_from_this<Waypoint>
			{
			public:
				/// <summary>
				/// The position of the waypoint
				/// </summary>				
				Position pos;

				/// <summary>
				/// The revision number.
				/// </summary>
				int revision;

				/// <summary>
				/// Radius of the waypoint
				/// </summary>				
				Real radius;

				/// <summary>
				/// The leg extending from the waypoint
				/// </summary>
				std::shared_ptr<Types::Leg> leg;

				/// <summary>
				/// A pointer to including waypoint set.
				/// </summary>
				std::weak_ptr<Route> route;

				using Ptr = std::shared_ptr<Waypoint>;
				std::shared_ptr<Waypoint> GetThis()
				{
					return shared_from_this();
				}				
			};

			class Waypoints : public ComponentBase, public std::vector<Types::Waypoint>
			{
			public:
				/// <summary>
				/// A pointer to including route
				/// </summary>				
				std::weak_ptr<Types::Route> route;

				using Ptr = std::shared_ptr<Waypoints>;

			public:
				XYList ToXYList() const; 
				void FromXYList(XYList xylist);
			};			
			
		}
	}
}

#endif
