#ifndef HASHCOLON_FELINE_TYPES_PREBUILTMAP_H
#define HASHCOLON_FELINE_TYPES_PREBUILTMAP_H

#include <HashColon/Feline/Feline_config.h>
#include <HashColon/Feline/GeoValues.hpp>

namespace HashColon::Feline
{
	struct PreBuiltMap
	{
		/*Mon(Types::Real w, Types::XYList iRoute)
		{
		weight = w;
		}

		Mon(Types::XYList iRoute)
		{
		weight = 1.0;
		route = iRoute;
		}*/

		/*Mon MergeWith(Mon other);
		std::vector<Types::Real> DifferenceWith(Mon other);

		Types::XYList NormalizedRoute();
		Types::Real RouteLength();*/

		int tribeNo;
		Real weight;
		XYList route;
		Eigen::MatrixXR covMatrix;
	};
}

#endif 
