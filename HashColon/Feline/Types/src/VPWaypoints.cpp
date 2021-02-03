#include <HashColon/Feline/Feline_config.h>

//#include <cassert>
#include <algorithm>

#include <HashColon/Helper/Helper.hpp>

#include <HashColon/Feline/Types/VPBase.hpp>
#include <HashColon/Feline/Types/VoyageSimple.hpp>
//#include <HashColon/Feline/Types/VPVoyage.hpp>
//#include <HashColon/Feline/Types/VPRoute.hpp>
//#include <HashColon/Feline/Types/VPSchedules.hpp>
//#include <HashColon/Feline/Types/VPMission.hpp>
#include <HashColon/Feline/Types/VPWaypoints.hpp>


namespace HashColon
{
	namespace Feline
	{
		namespace Types
		{
			Simple::XYList Waypoints::ToXYList() const
			{
				Simple::XYList re;
				for (Waypoint w : (*this))
				{
					re.push_back(w.pos);
				}
				return re;
			}

			void Waypoints::FromXYList(Simple::XYList xylist)
			{
				this->clear();
				for (Position p : xylist)
				{
					Waypoint tmp;
					tmp.pos = p;
					this->push_back(tmp);
				}				
			}

		}
	}
}