#ifndef HASHCOLON_FELINE_TYPES_PREBUILTMAP_H
#define HASHCOLON_FELINE_TYPES_PREBUILTMAP_H

#include <HashColon/Feline/Feline_config.h>
#include <HashColon/Feline/Types/ValueTypes.hpp>
#include <HashColon/Feline/Types/VoyageSimple.hpp>

namespace HashColon
{
	namespace Feline
	{
		namespace Types
		{
			struct PreBuiltMap
			{
				/*Mon(Types::Real w, Types::Simple::XYList iRoute)
				{
				weight = w; 
				}

				Mon(Types::Simple::XYList iRoute)
				{
				weight = 1.0;
				route = iRoute;
				}*/

				/*Mon MergeWith(Mon other);
				std::vector<Types::Real> DifferenceWith(Mon other);

				Types::Simple::XYList NormalizedRoute();
				Types::Real RouteLength();*/

				int tribeNo;
				Real weight;
				Simple::XYList route;
				Eigen::MatrixXR covMatrix;
			};
		}
	}
}

#endif 
