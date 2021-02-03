#ifndef HASHCOLON_FELINE_TYPES_AISDATA_HPP
#define HASHCOLON_FELINE_TYPES_AISDATA_HPP

#include <string>
#include <vector>
#include <map>
#include <HashColon/Helper/Exception.hpp>
#include <HashColon/Feline/Types/ValueTypes.hpp>

namespace HashColon::Feline::Types
{
	class AisDynamicRow
	{
	public:
		enum struct column { MMSI, datetime, longitude, latitude, COG, SOG, heading };

		HASHCOLON_NAMED_EXCEPTION_DEFINITION(DynamicColumnItem);

	protected:
		static const std::unordered_map<int, std::string> _column_toString;
		static const std::unordered_map<std::string, int> _column_toInt;
		friend std::istream& operator>>(std::istream &in, HashColon::Feline::Types::AisDynamicRow::column& c);
		friend std::ostream& operator<<(std::ostream &out, const HashColon::Feline::Types::AisDynamicRow::column& c);

	protected:
		int MMSI;
		TimePoint datetime;
		LongitudeType longitude;
		LatitudeType latitude;
		Degree COG;
		UnsignedReal SOG;
		Degree heading;

	public:
		int get_MMSI();
		std::string get_MMSI_str();
		void set_MMSI(int iMMSI);
		void set_MMSI(std::string iMMSI_str);

		HashColon::Feline::Types::TimePoint get_datetime();
		std::string get_datetime_str(const std::string formatStr);
		void set_datetime(HashColon::Feline::Types::TimePoint tp);
		void set_datetime(std::string idatetime, const std::string formatStr);


		double get_longitude();
		void set_longitude(double ilongitude);
		void set_longitude(std::string ilongitude_str);

		double get_latitude();
		void set_latitude(double ilatitude);
		void set_latitude(std::string ilatitude_str);

		double get_COG();
		void set_COG(double iCOG);
		void set_COG(std::string iCOG_str);

		double get_SOG();
		void set_SOG(double iSOG);
		void set_SOG(std::string iSOG_str);

		double get_heading();
		void set_heading(double iheading);
		void set_heading(std::string iheading_str);

		void from_string(std::string rowline_str, std::vector<column> columns, const std::string datetimeFormatStr);
		std::string to_string(std::vector<column> columnformat, const std::string datetimeFormatStr);

	};

	class AisStaticRow
	{
	public:
		enum struct column { MMSI, shipname, shiptypecode, nationalitycode, IMO, callsign, fore, aft, portside, starboard, draught };

	protected:
		static const std::unordered_map<int, std::string> _column_toString;
		static const std::unordered_map<std::string, int> _column_toInt;
		friend std::istream& operator>>(std::istream &in, HashColon::Feline::Types::AisStaticRow::column& c);
		friend std::ostream& operator<<(std::ostream &out, const HashColon::Feline::Types::AisStaticRow::column& c);

	protected:
		int MMSI;
		std::string shipname;
		int shiptypecode;
		int nationalitycode;
		int IMO;
		std::string callsign;
		std::vector<double> dimension = { 0.0, 0.0, 0.0, 0.0, 0.0 };
	public:
		int get_MMSI();
		std::string get_MMSI_str();
		void set_MMSI(int iMMSI);
		void set_MMSI(std::string iMMSI_str);

		std::string get_shipname();
		void set_shipname(std::string ishipname);

		int get_shiptypecode();
		void set_shiptypecode(int ishiptypecode);
		void set_shiptypecode(std::string ishiptypecode_str);

		int get_nationalitycode();
		void set_nationalitycode(int inationalitycode);
		void set_nationalitycode(std::string inationalitycode_str);

		int get_IMO();
		std::string get_IMO_str();
		void set_IMO(int iIMO);
		void set_IMO(std::string iIMO_str);

		std::string get_callsign();
		void set_callsign(std::string icallsign);

		std::vector<double>& get_dimension();
		void set_dimension(std::vector<double> idimension, bool checkformat = false);
		void set_forelen(double iforelen);
		void set_forelen(std::string iforelen_str);
		void set_aftlen(double iaftlen);
		void set_aftlen(std::string iaftlen_str);
		void set_portsidelen(double iportsidelen);
		void set_portsidelen(std::string iportsidelen_str);
		void set_starboardlen(double istarboardlen);
		void set_starboardlen(std::string istarboardlen_str);
		void set_draught(double idraught);
		void set_draught(std::string idraught_str);

		void from_string(std::string rowline_str, std::vector<column> columns);
		std::string to_string(std::vector<column> columnformat);

	public:
		AisStaticRow();
		AisStaticRow(std::string rowline_str, std::vector<column> columnformats);

	};

	class AisStaticDictionary : public std::map<int, AisStaticRow>
	{
	public:
		void insert_row(AisStaticRow iRow);
	};

	class AisDynamicTable : public std::vector<AisDynamicRow>
	{

	};

	std::istream& operator>>(std::istream &in, HashColon::Feline::Types::AisStaticRow::column& c);
	std::ostream& operator<<(std::ostream &out, const HashColon::Feline::Types::AisStaticRow::column& c);
	std::istream& operator>>(std::istream &in, HashColon::Feline::Types::AisDynamicRow::column& c);
	std::ostream& operator<<(std::ostream &out, const HashColon::Feline::Types::AisDynamicRow::column& c);
}


#endif