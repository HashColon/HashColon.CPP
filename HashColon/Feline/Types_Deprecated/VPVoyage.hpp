#ifndef HASHCOLON_FELINE_TYPES_VPVOYAGE_HPP
#define HASHCOLON_FELINE_TYPES_VPVOYAGE_HPP

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
			class Voyage : public NamedComponentBase, public std::enable_shared_from_this<Voyage>
			{
			public:
				/// <summary>
				/// The voyage routes
				/// </summary>
				std::vector<std::shared_ptr<Route>> voyageRoutes;

				/// <summary>
				/// The voyage schedules
				/// </summary>
				std::vector<std::shared_ptr<Schedules>> voyageSchedules;

				/// <summary>
				/// The vessel running the voyage
				/// </summary>
				std::shared_ptr<Vessel> vessel;

				/// <summary>
				/// The mission of the voyage
				/// </summary>
				std::shared_ptr<Mission> mission;

				/// <summary>
				/// The pointer to the active route
				/// </summary>
				std::weak_ptr<Route> activeRoute;

				/// <summary>
				/// The notes added to the voyage
				/// </summary>
				Notes notes;

				using Ptr = std::shared_ptr<Voyage>;
				
				std::shared_ptr<Voyage> GetThis()
				{
					return shared_from_this();
				}

			public:
				void Add_fromXYTList(Simple::XYTList xytlist);
				void Add_fromXYVVaTList(Simple::XYVVaTList xytvvalist);

				void RepairLinks();
			};

		}
	}
}

#endif 
