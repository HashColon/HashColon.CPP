#ifndef HASHCOLON_FELINE_TYPES_VPSCHEDULES_HPP
#define HASHCOLON_FELINE_TYPES_VPSCHEDULES_HPP

#include <HashColon/Feline/Feline_config.h>
#include <memory>
#include <HashColon/Feline/Types/ValueTypes.hpp>
#include <HashColon/Feline/Types/VPBase.hpp>

namespace HashColon
{
	namespace Feline
	{
		namespace Types
		{
			class Schedules : public ComponentBase, public std::vector<std::shared_ptr<Types::Schedule>>
			{
			public:
				/// <summary>
				/// A pointer to corresponding voyage.
				/// </summary>				
				std::weak_ptr<Types::Voyage> voyage;

				/// <summary>
				/// A pointer to corresponding route.
				/// </summary>				
				std::weak_ptr<Types::Route> route;

				// pointer type definition
				using Ptr = std::shared_ptr<Schedules>;

			public:
				void RepairLinks();

			};

			/// <summary>
			/// A schedule of the route. A route can have multiple schedules for a set of waypoints
			/// </summary>
			/// <seealso cref="NamedComponentBase" />
			/// <seealso cref="std::vector{Types::ScheduleElement}" />
			class Schedule : 
				public NamedComponentBase, public std::vector<Types::ScheduleElement>, public std::enable_shared_from_this<Schedule>
			{
			public:
				/// <summary>
				/// a pointer to related route
				/// </summary>				
				std::weak_ptr<Types::Route> route;

				// pointer type definition
				using Ptr = std::shared_ptr<Schedule>;
								
				std::shared_ptr<Schedule> GetThis()
				{
					return shared_from_this();
				}


			public:
				void RepairLinks();


			};

			/// <summary>
			/// A schedule element which defines a timestamp for a waypoint.
			/// </summary>
			/// <seealso cref="NamedComponentBase" />
			class ScheduleElement : public NamedComponentBase, public std::enable_shared_from_this<ScheduleElement>
			{
			public:
				/// <summary>
				/// The time window for etd
				/// </summary>
				Types::ValueInterval<Types::TimePoint> etd;

				/// <summary>
				/// The time window for eta
				/// </summary>
				Types::ValueInterval<Types::TimePoint> eta;

				/// <summary>
				/// The staying duration at the waypoint
				/// </summary>
				Types::Duration stay;

				/// <summary>
				/// The note
				/// </summary>
				Types::Notes note;

				/// <summary>
				/// The corresponding waypoint of the schedule element
				/// </summary>
				/// a pointer to linked waypoint
				std::weak_ptr<Types::Waypoint> waypoint;

				/// <summary>
				/// A pointer to the including schedule
				/// </summary>				
				std::weak_ptr<Types::Schedule> schedule;

				// pointer type definition
				using Ptr = std::shared_ptr<ScheduleElement>;
								
				std::shared_ptr<ScheduleElement> GetThis()
				{
					return shared_from_this();
				}
			};
		}
	}
}
#endif