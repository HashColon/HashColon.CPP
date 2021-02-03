#include <HashColon/Feline/Feline_config.h>

//#include <cassert>
#include <algorithm>

#include <HashColon/Helper/Helper.hpp>

#include <HashColon/Feline/Types/VPBase.hpp>
#include <HashColon/Feline/Types/VoyageSimple.hpp>
#include <HashColon/Feline/Types/VPVoyage.hpp>
#include <HashColon/Feline/Types/VPRoute.hpp>
#include <HashColon/Feline/Types/VPSchedules.hpp>
#include <HashColon/Feline/Types/VPMission.hpp>
#include <HashColon/Feline/Types/VPWaypoints.hpp>


namespace HashColon
{
	namespace Feline
	{
		namespace Types
		{
			// VPVoyage			
			void Voyage::Add_fromXYTList(Simple::XYTList xytlist)
			{
				this->voyageRoutes.push_back(std::make_shared<Types::Route>());

				this->voyageRoutes.back()->FromXYTList(xytlist);

				this->activeRoute = this->voyageRoutes.back();
			}

			void Voyage::Add_fromXYVVaTList(Simple::XYVVaTList xyvvatlist)
			{
				// TODO: temporary solution need to be fixed
				Add_fromXYTList(xyvvatlist.ToXYTList());
			}

			void Voyage::RepairLinks()
			{
				// repair links of routes
				for(std::shared_ptr<Route> route : voyageRoutes)
				{
					// if voyage link of each route is not this, correct it.					
					if(route->voyage.expired())
						route->voyage = this->GetThis();
					else if (route->voyage.lock() != this->GetThis())
						route->voyage = this->GetThis();

					// repair links of routes
					route->RepairLinks();
				}

				// repair links of schedules
				for(std::shared_ptr<Schedules> schedules : voyageSchedules)
				{
					// if voyage link of each schedules is not this, correct it.
					if (schedules->voyage.expired())
						schedules->voyage = this->GetThis();
					else if (schedules->voyage.lock() != this->GetThis())
						schedules->voyage = this->GetThis();

					// repair links of schedules
					schedules->RepairLinks();
				}

				// repair links of mission
				if(mission->voyage.expired())
					mission->voyage = this->GetThis();
				else if (mission->voyage.lock() != this->GetThis())
					mission->voyage = this->GetThis();

				// if activeRoute is null or wrong, define it as the first voyage route
				// if there is no voyage routes, there is no active route
				if (voyageRoutes.size() > 0) 
				{
					// check if activeRoute has no ref
					if(activeRoute.expired())
						activeRoute = voyageRoutes[0];
					// check if activeRoute is not in voyageRoutes
					else if (std::find(voyageRoutes.begin(), voyageRoutes.end(), activeRoute.lock())
						!= voyageRoutes.end())
						activeRoute = voyageRoutes[0];					
				}
			}

		}
	}
}
