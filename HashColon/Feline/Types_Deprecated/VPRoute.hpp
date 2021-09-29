#ifndef HASHCOLON_FELINE_TYPES_VPROUTE_HPP
#define HASHCOLON_FELINE_TYPES_VPROUTE_HPP

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
			enum RouteStatusEnum
			{
				/// <summary>
				/// The active route. A voyage should have only one active route.
				/// </summary>
				Active,

				/// <summary>
				/// The planned route. There can be many plans: plan A, plan B, .... etc.
				/// </summary>
				Planned,

				/// <summary>
				/// The deprecated route.
				/// </summary>
				Deprecated
			};

			class Route : public NamedComponentBase, public std::enable_shared_from_this<Route>
			{
			public:
				/// <summary>
				/// The author
				/// </summary>				
				std::string author;

				/// <summary>
				/// The status of the route
				/// </summary>
				RouteStatusEnum status;

				/// <summary>
				/// The notes for the route
				/// </summary>
				Notes notes;

				/// <summary>
				/// The revision history
				/// </summary>				
				Notes revisionHistory;

				/// <summary>
				/// The waypoints of the route.
				/// </summary>
				std::shared_ptr<Waypoints> waypoints;

				/// <summary>
				/// The schedules for the route and its waypoints
				/// </summary>
				std::shared_ptr<Schedules> schedules;

				/// <summary>
				/// The cost assessment result of the route
				/// </summary>
				// TODO: VoyageCosts disabled 
				//std::shared_ptr<QRA::VoyageCosts> costAssessment;

				/// <summary>
				/// A pointer to the voyage
				/// </summary>
				std::weak_ptr<Voyage> voyage;

				// pointer type definition
				using Ptr = std::shared_ptr<Route>;
								
				std::shared_ptr<Route> GetThis()
				{
					return shared_from_this();
				}

			public:
				void Add_newWaypoints();
				void Add_newWaypoints(Waypoints waypoints);
				void Add_newWaypoints(std::shared_ptr<Waypoints> waypointsPtr);				
				void Add_newSchedules(Schedules schedules);
				void Add_newSchedules(std::shared_ptr<Schedules> schedulesPtr);				
				void Add_newSchedule(Schedule schedule);				

				XYList ToXYList();
				Simple::XYTList ToXYTList(int idx);
				Simple::XYVVaTList ToXYVVaTList(int idx) const;

				void FromXYTList(Simple::XYTList xytlist);				

				void RepairLinks();

			protected:
				void Validate_NumberOfWaypointsAndScheduleCoincide();
				void Validate_NumberOfWaypointsAndScheduleCoincide(Waypoints& waypoints, Schedules& schedules);
				void Validate_NumberOfWaypointsAndScheduleCoincide(Waypoints& waypoints, Schedule& schedule);				

			public:

			};



		}
	}
}
#endif

