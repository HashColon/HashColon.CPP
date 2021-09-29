#ifndef HASHCOLON_FELINE_TYPES_VPMISSION_HPP
#define HASHCOLON_FELINE_TYPES_VPMISSION_HPP

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

			class CargoInfo;
			class CrewInfo;
			class Port;
			class Voyage;

			/// <summary>
			/// Mission is the variant properties which varies by voyages.
			/// </summary>
			/// <seealso cref="NamedComponentBase" />
			class Mission : public NamedComponentBase, public std::enable_shared_from_this<Mission>
			{
			public:
				/// <summary>
				/// The cargo information
				/// </summary>
				std::shared_ptr<CargoInfo> cargo;

				/// <summary>
				/// The crews information
				/// </summary>
				std::vector<std::shared_ptr<CrewInfo>> crews;

				/// <summary>
				/// The origin point of the voyage
				/// </summary>
				Position originPoint;

				/// <summary>
				/// The origin port of the voyage. This can be null if the origin is not a port.
				/// </summary>
				std::shared_ptr<Port> originPort;

				/// <summary>
				/// The destination point of the voyage
				/// </summary>
				Position destinationPoint;

				/// <summary>
				/// The destination port of the voyage. This can be null if the destination is not a port.
				/// </summary>
				std::shared_ptr<Port> destinationPort;

				/// <summary>
				/// A pointer to the voyage
				/// </summary>
				std::weak_ptr<Voyage> voyage;

				// pointer type definition
				using Ptr = std::shared_ptr<Mission>;
				std::shared_ptr<Mission> GetThis()
				{
					return shared_from_this();
				}
			};
		}
	}
}


#endif