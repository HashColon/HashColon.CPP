#ifndef HASHCOLON_FELINE_TYPES_VPBASE_HPP
#define HASHCOLON_FELINE_TYPES_VPBASE_HPP

#include <HashColon/Feline/Feline_config.h>
#include <memory>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <HashColon/Feline/ValueTypes.hpp>

namespace HashColon
{
	namespace Feline
	{
		namespace Types
		{
			using ComponentNamespace = std::string;
			// TODO: the use of database uuid is unspecified
			using ComponentID = boost::uuids::uuid;
			//using ComponentID = std::string;

			/// <summary>
			/// Abstract class for RTX extension.
			/// </summary>
			class ComponentExtension
			{
			public:
				using Ptr = std::shared_ptr<ComponentExtension>;

				/// <summary>
				/// The manufacturer
				/// </summary>
				std::string manufacturer;
				/// <summary>
				/// The name
				/// </summary>
				std::string name;
				/// <summary>
				/// The version
				/// </summary>
				std::string version;
				/// <summary>
				/// The data
				/// </summary>
				VoidPtr data;
			};

			/// <summary>
			/// Abstract class which defines component with id
			/// </summary>
			class ComponentBase
			{
			public:
				/// <summary>
				/// The identifier namespace
				/// </summary>
				ComponentNamespace idNamespace;
				/// <summary>
				/// The universally unique identifier for DB
				/// </summary>
				ComponentID id;
				/// <summary>
				/// The extension defined in RTX format
				/// </summary>
				ComponentExtension::Ptr extension;

			protected:
				/**
			   * @brief private constructor for ComponentBase
			   *
			   */
				ComponentBase()
				{
					idNamespace = "";
					//id = "";
				}

			public:
				ComponentBase(const ComponentBase &) = default;

				inline bool operator==(const ComponentBase &rhs)
				{
					return (idNamespace == rhs.idNamespace) && (id == rhs.id);
				};

				inline bool operator!=(const ComponentBase &rhs) { return !(*this == rhs); };

				std::string id_str() { return boost::uuids::to_string(id); };

				void id_str(std::string uuid_string) { id = boost::uuids::string_generator()(uuid_string);};
			};

			/// <summary>
			/// Abstract class for components which has names
			/// </summary>
			/// <seealso cref="ComponentBase" />
			class NamedComponentBase : public ComponentBase
			{
			public:
				/// <summary>
				/// The name
				/// </summary>
				std::string name;
				/// <summary>
				/// The valid period of the component
				/// </summary>
				ValueInterval<TimePoint> ValidPeriod;
				/// <summary>
				/// The issued date of the component
				/// </summary>
				TimePoint issuedDate;

			protected:
				NamedComponentBase() : ComponentBase() {};

			public:
				NamedComponentBase(const NamedComponentBase &) = default;
			};

			/// <summary>
			/// Base class for notes added in voyage plans.
			/// </summary>
			struct Note
			{
				/// <summary>
				/// The issued date
				/// </summary>
				TimePoint issuedDate;
				/// <summary>
				/// The note
				/// </summary>
				std::string note;
			};

			/// <summary>
			/// A set of Notes.
			/// </summary>
			/// <seealso cref="std::vector{Types::Note}" />
			struct Notes : public std::vector<Note>
			{
			};

			class Voyage;
			class Mission;
			class Route;
			class Vessel;			
			class Waypoints;
			class Waypoint;
			class Schedules;
			class Schedule;
			class ScheduleElement;
			
		}
	}
}

#endif