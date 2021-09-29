#include <unordered_map>
#include <HashColon/Feline/GeoValues.hpp>
#include <HashColon/Feline/Types/AisData.hpp>


using namespace std;
using namespace HashColon::Feline::Types;

int AisDynamicRow::get_MMSI() { return MMSI; }
string AisDynamicRow::get_MMSI_str() { return string(9 - std::to_string(MMSI).length(), '0') + std::to_string(MMSI); }
void AisDynamicRow::set_MMSI(int iMMSI)
{
	// check number of digit; check positiveness
	if (iMMSI < 0 || iMMSI >= 1000000000)
		throw std::exception();
	MMSI = iMMSI;
}
void AisDynamicRow::set_MMSI(string iMMSI_str)
{
	int tmpMMSI = 0;
	size_t tmpflag;
	try
	{
		tmpMMSI = stoi(iMMSI_str, &tmpflag);
		if (tmpflag != iMMSI_str.size())
			tmpMMSI = 0;
		else if (iMMSI_str.length() > 9)
			tmpMMSI = 0;
	}
	catch (invalid_argument&) { tmpMMSI = 0; }
	catch (out_of_range&) { tmpMMSI = 0; }

	// if the tmpMMSI is 0 or negative, this is error
	if (tmpMMSI <= 0) throw std::exception();

	set_MMSI(tmpMMSI);
}

TimePoint AisDynamicRow::get_datetime() { return datetime; }
string AisDynamicRow::get_datetime_str(const std::string formatStr) { return datetime.toString(formatStr); }
void AisDynamicRow::set_datetime(HashColon::Feline::Types::TimePoint tp) { datetime = tp; }
void AisDynamicRow::set_datetime(std::string idatetime, const std::string formatStr) { datetime.fromString(idatetime, formatStr); }

double AisDynamicRow::get_longitude() { return longitude; }

void AisDynamicRow::set_longitude(double ilongitude)
{
	HashColon::Feline::Types::LongitudeType tmplon = ilongitude;
	longitude = tmplon;
}

void AisDynamicRow::set_longitude(string ilongitude_str)
{
	set_longitude(stod(ilongitude_str));
}

double AisDynamicRow::get_latitude() { return latitude; }

void AisDynamicRow::set_latitude(double ilatitude)
{
	HashColon::Feline::Types::LatitudeType tmplat = ilatitude;
	latitude = tmplat;
}

void AisDynamicRow::set_latitude(string ilatitude_str)
{
	set_latitude(stod(ilatitude_str));
}

double AisDynamicRow::get_COG() { return COG; }

void AisDynamicRow::set_COG(double iCOG)
{
	HashColon::Feline::Types::Degree tmpcog = iCOG;
	COG = tmpcog;
}

void AisDynamicRow::set_COG(string iCOG_str)
{
	set_COG(stod(iCOG_str));
}

double AisDynamicRow::get_SOG() { return SOG; }

void AisDynamicRow::set_SOG(double iSOG)
{
	// speed is non-negative
	if (iSOG < 0) iSOG = 0;
	SOG = iSOG;
}

void AisDynamicRow::set_SOG(string iSOG_str)
{
	set_SOG(stod(iSOG_str));
}

double AisDynamicRow::get_heading() { return heading; }

void AisDynamicRow::set_heading(double iheading)
{
	HashColon::Feline::Types::Degree tmpheading = iheading;
	heading = tmpheading;
}

void AisDynamicRow::set_heading(string iheading_str)
{
	set_heading(stod(iheading_str));
}

void AisDynamicRow::from_string(string rowline_str, vector<column> columns, const string datetimeFormatStr)
{
	// there should be 'size-of-columns - 1' commas in a line
	if (count(rowline_str.begin(), rowline_str.end(), ',') != (columns.size() - 1))
		throw std::exception();

	istringstream iss(rowline_str);
	string buffer;

	for (column& c : columns)
	{
		buffer.clear();
		getline(iss, buffer, ',');

		switch (c)
		{
		case column::MMSI:
			set_MMSI(buffer); break;
		case column::datetime:
			set_datetime(buffer, datetimeFormatStr); break;
		case column::longitude:
			set_longitude(buffer); break;
		case column::latitude:
			set_latitude(buffer); break;
		case column::COG:
			set_COG(buffer); break;
		case column::SOG:
			set_SOG(buffer); break;
		case column::heading:
			set_heading(buffer); break;
		default:
			throw std::exception();
		}
	}
}

string AisDynamicRow::to_string(vector<column> columnformat, const string datetimeFormatStr)
{
	string re = ""; re.clear();

	for (column& aitem : columnformat)
	{
		string segment = "";  segment.clear();

		switch (aitem)
		{
		case column::MMSI:
			segment = get_MMSI_str(); break;
		case column::datetime:
			segment = get_datetime_str(datetimeFormatStr); break;
		case column::longitude:
			segment = std::to_string(get_longitude()); break;
		case column::latitude:
			segment = std::to_string(get_latitude()); break;
		case column::COG:
			segment = std::to_string(get_COG()); break;
		case column::SOG:
			segment = std::to_string(get_SOG()); break;
		case column::heading:
			segment = std::to_string(get_heading()); break;
		default:
			throw std::exception();
		}

		re = re + segment;

		if (&aitem != &columnformat.back())
			re = re + ',';
	}

	return re;
}

int AisStaticRow::get_MMSI() { return MMSI; }
string AisStaticRow::get_MMSI_str() { return string(9 - std::to_string(MMSI).length(), '0') + std::to_string(MMSI); }
void AisStaticRow::set_MMSI(int iMMSI)
{
	// check number of digit; check positiveness
	if (iMMSI < 0 || iMMSI >= 1000000000)
		MMSI = 0;
	else 
		MMSI = iMMSI;
}
void AisStaticRow::set_MMSI(string iMMSI_str)
{
	int tmpMMSI = 0;
	size_t tmpflag;
	try
	{
		tmpMMSI = stoi(iMMSI_str, &tmpflag);
		if (tmpflag != iMMSI_str.size())
			tmpMMSI = 0;
		else if (iMMSI_str.length() > 9)
			tmpMMSI = 0;
	}
	catch (invalid_argument&) { tmpMMSI = 0; }
	catch (out_of_range&) { tmpMMSI = 0; }

	// if the tmpMMSI is 0 or negative, this is error
	if (tmpMMSI <= 0) throw std::exception();

	set_MMSI(tmpMMSI);
}

string AisStaticRow::get_shipname() { return shipname; }
void AisStaticRow::set_shipname(string ishipname) { shipname = ishipname; }

int AisStaticRow::get_shiptypecode() { return shiptypecode; }
void AisStaticRow::set_shiptypecode(int ishiptypecode) { shiptypecode = ishiptypecode; }
void AisStaticRow::set_shiptypecode(string ishiptypecode_str)
{
	try { set_shiptypecode(stoi(ishiptypecode_str)); }
	catch (...) { set_shiptypecode(0); }
}

int AisStaticRow::get_nationalitycode() { return nationalitycode; }
void AisStaticRow::set_nationalitycode(int inationalitycode) { nationalitycode = inationalitycode; }
void AisStaticRow::set_nationalitycode(string inationalitycode_str)
{
	try { set_nationalitycode(stoi(inationalitycode_str)); }
	catch (...) { set_nationalitycode(0); }
}

int AisStaticRow::get_IMO() { return IMO; }
string AisStaticRow::get_IMO_str() { return string(7 - std::to_string(IMO).length(), '0') + std::to_string(IMO); }
void AisStaticRow::set_IMO(int iIMO)
{
	// check number of digit; check positiveness
	if (iIMO < 0 || iIMO >= 10000000)
		IMO = 0;
	else
		IMO = iIMO;
}
void AisStaticRow::set_IMO(string iIMO_str)
{
	int imonum_int;
	size_t tmpflag;
	try
	{
		imonum_int = stoi(iIMO_str, &tmpflag);
		if (tmpflag != iIMO_str.size())
			imonum_int = 0;
		else if (iIMO_str.length() > 7)
			imonum_int = 0;
	}
	catch (invalid_argument&) { imonum_int = 0; }
	catch (out_of_range&) { imonum_int = 0; }

	set_IMO(imonum_int);
}

string AisStaticRow::get_callsign() { return callsign; }
void AisStaticRow::set_callsign(string icallsign) { callsign = icallsign; }

vector<double>& AisStaticRow::get_dimension() { return dimension; }
void AisStaticRow::set_dimension(vector<double> idimension, bool checkformat)
{
	if (checkformat)
	{
		if (idimension.size() != 5)
			throw std::exception();

		for (double& alen : idimension)
			if (alen < 0)
				alen = 0.0;
	}
	dimension = idimension;
}

void AisStaticRow::set_forelen(double iforelen)
{
	// assertions
	if (dimension.size() != 5) dimension.resize(5);
	if (iforelen < 0) iforelen = 0;

	dimension[0] = iforelen;
}
void AisStaticRow::set_forelen(string iforelen_str) 
{
	try { set_forelen(stod(iforelen_str)); }
	catch (...) { set_forelen(0); }
}

void AisStaticRow::set_aftlen(double iaftlen)
{
	// assertions
	if (dimension.size() != 5) dimension.resize(5);
	if (iaftlen < 0) iaftlen = 0;

	dimension[1] = iaftlen;
}

void AisStaticRow::set_aftlen(string iaftlen_str) 
{ 
	try { set_aftlen(stod(iaftlen_str)); }
	catch (...) { set_aftlen(0); }
}

void AisStaticRow::set_portsidelen(double iportsidelen)
{
	// assertions
	if (dimension.size() != 5) dimension.resize(5);
	if (iportsidelen < 0) iportsidelen = 0;

	dimension[2] = iportsidelen;
}

void AisStaticRow::set_portsidelen(string iportsidelen_str) 
{ 
	try { set_portsidelen(stod(iportsidelen_str)); }
	catch (...) { set_portsidelen(0); }
}

void AisStaticRow::set_starboardlen(double istarboardlen)
{
	// assertions
	if (dimension.size() != 5) dimension.resize(5);
	if (istarboardlen < 0) istarboardlen = 0;

	dimension[3] = istarboardlen;
}

void AisStaticRow::set_starboardlen(string istarboardlen_str) 
{ 
	try { set_starboardlen(stod(istarboardlen_str)); }
	catch (...) { set_starboardlen(0); }
}

void AisStaticRow::set_draught(double idraught)
{
	// assertions
	if (dimension.size() != 5) dimension.resize(5);
	if (idraught < 0) idraught = 0;

	dimension[4] = idraught;
}

void AisStaticRow::set_draught(string idraught_str) 
{
	try { set_draught(stod(idraught_str)); }
	catch (...) { set_draught(0); }
}


void AisStaticRow::from_string(string rowline_str, vector<column> columns)
{
	// there should be 'size-of-columns - 1' commas in a line
	if (count(rowline_str.begin(), rowline_str.end(), ',') != (columns.size() - 1))
		throw std::exception();

	istringstream iss(rowline_str);
	string buffer;

	for (column& c : columns)
	{
		buffer.clear();
		getline(iss, buffer, ',');

		switch (c)
		{
		case column::MMSI:
			set_MMSI(buffer); break;
		case column::shipname:
			set_shipname(buffer); break;
		case column::shiptypecode:
			set_shiptypecode(buffer); break;
		case column::nationalitycode:
			set_nationalitycode(buffer); break;
		case column::IMO:
			set_IMO(buffer); break;
		case column::callsign:
			set_callsign(buffer); break;
		case column::fore:
			set_forelen(buffer); break;
		case column::aft:
			set_aftlen(buffer); break;
		case column::portside:
			set_portsidelen(buffer); break;
		case column::starboard:
			set_starboardlen(buffer); break;
		case column::draught:
			set_draught(buffer); break;
		default:
			throw std::exception();
		}
	}
}

string AisStaticRow::to_string(vector<column> columnformat)
{
	string re = ""; re.clear();

	for (column& aitem : columnformat)
	{
		string segment = "";  segment.clear();

		switch (aitem)
		{
		case column::MMSI:
			segment = get_MMSI_str(); break;
		case column::shipname:
			segment = get_shipname(); break;
		case column::shiptypecode:
			segment = std::to_string(get_shiptypecode()); break;
		case column::nationalitycode:
			segment = std::to_string(get_nationalitycode()); break;
		case column::IMO:
			segment = get_IMO_str(); break;
		case column::callsign:
			segment = get_callsign(); break;
		case column::fore:
			segment = std::to_string(get_dimension()[0]); break;
		case column::aft:
			segment = std::to_string(get_dimension()[1]); break;
		case column::portside:
			segment = std::to_string(get_dimension()[2]); break;
		case column::starboard:
			segment = std::to_string(get_dimension()[3]); break;
		case column::draught:
			segment = std::to_string(get_dimension()[4]); break;
		default:
			throw std::exception();
		}

		re = re + segment;

		if (&aitem != &columnformat.back())
			re = re + ',';
	}

	return re;
}

AisStaticRow::AisStaticRow() {}
AisStaticRow::AisStaticRow(string rowline_str, vector<column> columnformats) { from_string(rowline_str, columnformats); }


void AisStaticDictionary::insert_row(AisStaticRow iRow)
{
	int key = iRow.get_MMSI();

	// if no keys found, then insert. else ignore.
	if (find(key) == end())
		insert(make_pair(iRow.get_MMSI(), iRow));
}

// string-to-int converter / int-to-string converter for AisStaticRow
const unordered_map<int, string> AisDynamicRow::_column_toString =
{
	{static_cast<int>(column::MMSI), "MMSI"},
	{static_cast<int>(column::datetime), "datetime"},
	{static_cast<int>(column::longitude), "longitude"},
	{static_cast<int>(column::latitude), "latitude"},
	{static_cast<int>(column::COG), "COG"},
	{static_cast<int>(column::SOG), "SOG"},
	{static_cast<int>(column::heading), "heading"}
};
const unordered_map<string, int> AisDynamicRow::_column_toInt =
{
	{"MMSI", static_cast<int>(column::MMSI)},
	{"datetime", static_cast<int>(column::datetime)},
	{"longitude", static_cast<int>(column::longitude)},
	{"latitude", static_cast<int>(column::latitude)},
	{"COG", static_cast<int>(column::COG)},
	{"SOG", static_cast<int>(column::SOG)},
	{"heading", static_cast<int>(column::heading)}
};

const unordered_map<int, string> AisStaticRow::_column_toString = {
	{static_cast<int>(column::MMSI), "MMSI"},
	{static_cast<int>(column::shipname), "shipname"},
	{static_cast<int>(column::shiptypecode), "shiptypecode"},
	{static_cast<int>(column::nationalitycode), "nationalitycode"},
	{static_cast<int>(column::IMO), "IMO"},
	{static_cast<int>(column::callsign), "callsign"},
	{static_cast<int>(column::fore), "fore"},
	{static_cast<int>(column::aft), "aft"},
	{static_cast<int>(column::portside), "portside"},
	{static_cast<int>(column::starboard), "starboard"},
	{static_cast<int>(column::draught), "draught"}
};
const unordered_map<string, int> AisStaticRow::_column_toInt = {
	{"MMSI", static_cast<int>(column::MMSI)},
	{"shipname", static_cast<int>(column::shipname)},
	{"shiptypecode", static_cast<int>(column::shiptypecode)},
	{"nationalitycode", static_cast<int>(column::nationalitycode)},
	{"IMO", static_cast<int>(column::IMO)},
	{"callsign", static_cast<int>(column::callsign)},
	{"fore", static_cast<int>(column::fore)},
	{"aft", static_cast<int>(column::aft), },
	{"portside", static_cast<int>(column::portside)},
	{"starboard", static_cast<int>(column::starboard)},
	{"draught", static_cast<int>(column::draught)}
};



istream& HashColon::Feline::Types::operator>>(istream &in, AisStaticRow::column& c)
{
	string str; in >> str;
	int i = AisStaticRow::_column_toInt.at(str);
	c = static_cast<AisStaticRow::column>(i);
	return in;
}
ostream& HashColon::Feline::Types::operator<<(ostream &out, const AisStaticRow::column& c)
{
	return out << AisStaticRow::_column_toString.at(static_cast<int>(c));
}
istream& HashColon::Feline::Types::operator>>(istream &in, AisDynamicRow::column& c)
{
	string str; in >> str;
	int i = AisDynamicRow::_column_toInt.at(str);
	c = static_cast<AisDynamicRow::column>(i);
	return in;
}
ostream& HashColon::Feline::Types::operator<<(ostream &out, const AisDynamicRow::column& c)
{
	return out << AisDynamicRow::_column_toString.at(static_cast<int>(c));
}

