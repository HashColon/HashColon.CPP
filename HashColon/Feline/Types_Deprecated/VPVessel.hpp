#ifndef HASHCOLON_FELINE_TYPES_VPVESSEL_HPP
#define HASHCOLON_FELINE_TYPES_VPVESSEL_HPP

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

			// concider this structure to be union-based
			struct VesselDimension
			{
				Real L;
				Real B;
				Real D;
				// Draught is not part of vessel dimension, but design draught is.
				Real designT;
				Real grossTonnage;
				Real netTonnage;
			};

			struct Propulsion;
			struct Resistance;
			struct Stability;
			struct SeaKeeping;
			struct Manoeuvrability;

			class Vessel : public NamedComponentBase, public std::enable_shared_from_this<Vessel>
			{
			public:
				/// <summary>
				/// The dimension of the vessel
				/// </summary>
				VesselDimension dimension;

				/// <summary>
				/// The propulsion performance
				/// </summary>
				std::shared_ptr<Propulsion> propulsion;

				/// <summary>
				/// The resistance traits
				/// </summary>
				std::shared_ptr<Resistance> resistance;

				/// <summary>
				/// The stability traits
				/// </summary>
				std::shared_ptr<Stability> stability;

				/// <summary>
				/// The seakeeping traits
				/// </summary>
				std::shared_ptr<SeaKeeping> seakeeping;

				/// <summary>
				/// The manoeuvrability traits
				/// </summary>
				std::shared_ptr<Manoeuvrability> manoeuvrability;

				/// <summary>
				/// The mmsi number of the ship
				/// </summary>
				std::string mmsi;

				/// <summary>
				/// The imo number
				/// </summary>
				std::string imoNumber;

				/// <summary>
				/// The call sign
				/// </summary>
				std::string callSign;

				// TODO: type of hullType & vesselType should be revised
				std::string hullType;
				std::string vesselType;

				std::string buildDate;
				std::string ownerFlag;
				std::string ownerName;
				std::string operatorFlag;
				std::string operatorName;
				std::string vesselFlag;
				std::string prevCallSign;
				std::string registeredPort;

				using Ptr = std::shared_ptr<Vessel>;
								
				std::shared_ptr<Vessel> GetThis()
				{
					return shared_from_this();
				}

			public:
				Vessel(const Vessel &) = default;
				// constructor from DB
				Vessel() : NamedComponentBase() {};

			public:
				inline bool
					operator==(const Vessel &rhs)
				{
					// if DB id is equal(this is mandatory) & one of {mmsi, imo number, callsign} coincide,
					// the vessel is equal.
					return (mmsi == rhs.mmsi || imoNumber == rhs.imoNumber || callSign == rhs.callSign) && ComponentBase::operator==(rhs);
				}

				inline bool operator!=(const Vessel &rhs)
				{
					return !(*this == rhs);
				}
			};
		}
	}
}

#endif