#include <HashColon/Feline/Feline_config.h>

#include <cassert>

#include <HashColon/Helper/Helper.hpp>

#include <HashColon/Feline/Types/VPBase.hpp>
#include <HashColon/Feline/Types/VoyageSimple.hpp>
#include <HashColon/Feline/Types/VPVoyage.hpp>
#include <HashColon/Feline/Types/VPRoute.hpp>
#include <HashColon/Feline/Types/VPWaypoints.hpp>
#include <HashColon/Feline/Types/VPSchedules.hpp>

namespace HashColon
{
	namespace Feline
	{
		namespace Types
		{
			void Route::Add_newWaypoints()
			{
				// drop off existing waypoints & schedules 
				// add a empty waypoints and a empty schedules
				// (scdeules are combined with waypoints. therefore it is nessesary to delete old schedules)
				this->waypoints = std::shared_ptr<Waypoints>(new Waypoints);
				this->schedules = std::shared_ptr<Schedules>(new Schedules);
				this->waypoints->clear(); this->schedules->clear();

				RepairLinks();
			}

			void Route::Add_newWaypoints(Waypoints waypoints)
			{
				// drop off existing waypoints & schedules 
				// add the given waypoints and a empty schedules
				// (scdeules are combined with waypoints. therefore it is nessesary to delete old schedules)
				this->waypoints = std::make_shared<Waypoints>(waypoints);
				this->schedules = std::shared_ptr<Schedules>(new Schedules);
				this->schedules->clear();

				RepairLinks();
			}

			void Route::Add_newWaypoints(std::shared_ptr<Waypoints> waypointsPtr)
			{
				// drop off existing waypoints & schedules 
				// add the given waypoints and a empty schedules
				// (scdeules are combined with waypoints. therefore it is nessesary to delete old schedules)
				this->waypoints = waypointsPtr;
				this->schedules = std::shared_ptr<Schedules>(new Schedules);
				this->schedules->clear();

				RepairLinks();
			}

			void Route::Add_newSchedules(Schedules schedules)
			{
				// Validate number of waypoints == number of schedule element
				Validate_NumberOfWaypointsAndScheduleCoincide((*(this->waypoints)), schedules);

				// drop off existing schedules and add the given schdules 
				this->schedules = std::make_shared<Schedules>(schedules);

				RepairLinks();
			}

			void Route::Add_newSchedules(std::shared_ptr<Schedules> schedulesPtr)
			{
				// Validate number of waypoints == number of schedule element
				Validate_NumberOfWaypointsAndScheduleCoincide((*(this->waypoints)), (*schedulesPtr));

				// drop off existing schedules and add the given schdules 
				this->schedules = schedulesPtr;

				RepairLinks();
			}
			
			void Route::Add_newSchedule(Schedule schedule)
			{
				// Validate number of waypoints == number of schedule element
				Validate_NumberOfWaypointsAndScheduleCoincide((*(this->waypoints)), schedule);

				this->schedules->push_back(std::make_shared<Schedule>(schedule));

				RepairLinks();
			}

			Simple::XYList Route::ToXYList()
			{
				return this->waypoints->ToXYList();
			}

			Simple::XYTList Route::ToXYTList(int idxOfSchedule)
			{
				// assert number of waypoints == number of schedule element
				Validate_NumberOfWaypointsAndScheduleCoincide();
				
				int n = this->waypoints->size();				
				Simple::XYTList re;
				re.clear(); re.reserve(n);
				for (int i = 0; i < n; i++)
				{
					Simple::XYT temp;
					temp.Pos = this->waypoints->at(i).pos;
					temp.TP = this->schedules->at(idxOfSchedule)->at(i).etd.repVal;
					re.push_back(temp);
				}
				return re;
			}

			Simple::XYVVaTList Route::ToXYVVaTList(int idx) const
			{
				throw NotImplementedException;
			}

			void Route::FromXYTList(Simple::XYTList xytlist)
			{
				// make a new waypoints and a new schedules
				std::shared_ptr<Waypoints> tempWaypoints(new Waypoints);
				std::shared_ptr<Schedules> tempSchedules(new Schedules);
				tempWaypoints->clear(); tempSchedules->clear();

				// make a new schedule
				Schedule tempSchedule;
				tempSchedule.clear();

				// for every xyt in given xytlist, 				
				for (Simple::XYT xyt : xytlist)
				{
					// make a waypoint and scheduleElement
					Waypoint tempWaypoint;
					ScheduleElement tempScheduleElement;

					tempWaypoint.pos = xyt.Pos;
					tempScheduleElement.etd[0]
						= tempScheduleElement.etd[1]
						= tempScheduleElement.etd[2]
						= xyt.TP;

					// add temp waypoint and scheduleElement
					tempWaypoints->push_back(tempWaypoint);
					tempSchedule.push_back(tempScheduleElement);
				}

				// add the temp schedule in schedules
				tempSchedules->push_back(std::make_shared<Schedule>(tempSchedule));

				// add tempWaypoints & tempSchedules to route
				this->Add_newWaypoints(tempWaypoints);
				this->Add_newSchedules(tempSchedules);
				
				// build links 
				RepairLinks();
			}

			void Route::Validate_NumberOfWaypointsAndScheduleCoincide()
			{
				// for every schedule,
				// validate number of waypoints == number of schedule element
				int n = this->waypoints->size();
				for (Schedule::Ptr s : (*(this->schedules)))
				{
					if (s->size() != n)
					{
						// TODO: throw exception. the exception for this case should be defined
						throw std::exception();
					}
				}
			}

			void Route::Validate_NumberOfWaypointsAndScheduleCoincide(Waypoints& waypoints, Schedule& schedule)
			{
				// validate number of waypoints == number of schedule element
				if (schedule.size() != waypoints.size())
				{
					// TODO: throw exception. the exception for this case should be defined
					throw std::exception();
				}
			}

			void Route::Validate_NumberOfWaypointsAndScheduleCoincide(Waypoints& waypoints, Schedules& schedules)
			{
				// for every schedule,
				// validate number of waypoints == number of schedule element

				int n = waypoints.size();
				for (Schedule::Ptr s : schedules)
				{
					if (s->size() != n)
					{
						// TODO: throw exception. the exception for this case should be defined
						throw std::exception();
					}
				}
			}

			void Route::RepairLinks()
			{
				// if waypoints exists,
				if (this->waypoints)
				{
					// check if the route link of the waypoints is this,
					if (this->waypoints->route.expired())
						this->waypoints->route = this->GetThis();
					else if (this->waypoints->route.lock() != this->GetThis())
						// if not, correct it.
						this->waypoints->route = this->GetThis();
				}

				// if schedules exists,
				if (this->schedules)
				{
					// check if the route link of the schedules is this
					if(this->schedules->route.expired())
						this->schedules->route = this->GetThis();
					else if (this->schedules->route.lock() != this->GetThis())
						// if not, correct it
						this->schedules->route = this->GetThis();

					// correct link of the schedules
					this->schedules->RepairLinks();

					// for each schedule in the schedules,
					for (Schedule::Ptr s : (*(this->schedules)))
					{
						// check if the route linke of each schedule is this
						if(s->route.expired())
							s->route = this->GetThis();
						else if (s->route.lock() != this->GetThis())
							s->route = this->GetThis();
					}
				}				
			}

		}
	}
}
