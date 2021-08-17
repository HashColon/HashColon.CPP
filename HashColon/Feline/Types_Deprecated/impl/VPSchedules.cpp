#include <HashColon/Feline/Feline_config.h>

//#include <cassert>
#include <algorithm>

#include <HashColon/Helper/Helper.hpp>

#include <HashColon/Feline/Types/VPBase.hpp>
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
			void Schedules::RepairLinks()
			{
				for (Schedule::Ptr s : (*this))
				{
					s->RepairLinks();
				}
			}

			void Schedule::RepairLinks()
			{
				for (ScheduleElement se : (*this))
				{
					if (se.schedule.expired())
						se.schedule = this->GetThis();
					else if (se.schedule.lock() != this->GetThis())
						se.schedule = this->GetThis();
				}
			}
		}
	}
}