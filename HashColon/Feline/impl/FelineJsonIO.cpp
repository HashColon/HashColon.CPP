// HashColon config
#include <HashColon/HashColon_config.h>
// std libraries
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
// dependant external libraries
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
// HashColon libraries
#include <HashColon/Helper.hpp>
#include <HashColon/Feline/ValueTypes.hpp>
// header file for this source file
#include <HashColon/Feline/FelineJsonIO.hpp>

using namespace std;
using namespace HashColon::Feline;
namespace Json = rapidjson;

// tags for FelineJson
namespace tag
{
	const char* Routes = "routes";

	const char* Static = "static";
	const char* IMO = "imo";
	const char* MMSI = "mmsi";
	const char* LOA = "loa";
	const char* Beam = "beam";
	const char* Draught = "draught";

	const char* Dynamic = "dynamic";
	const char* Lon = "longitude";
	const char* Lat = "latitude";
	const char* SOG = "sog";
	const char* COG = "cog";
	const char* XTD_P = "xtd_p";
	const char* XTD_S = "xtd_s";
	const char* Timestamp = "timestamp";
}

// Read Feline Json file
namespace HashColon::Feline::IO
{
	template <typename T>
	T ReadFromValue(Json::Value& val, const char* tag)
	{
		if (val.HasMember(tag))
			return val[tag].Get<T>();
		return 0.0;
	}

	template <>
	string ReadFromValue(Json::Value& val, const char* tag)
	{
		if (val.HasMember(tag))
			return val[tag].GetString();
		return "";
	}

	template <>
	bool ReadFromValue(Json::Value& val, const char* tag)
	{
		if (val.HasMember(tag))
		{
			string tfval = String::ToLowerCopy(val[tag].GetString());
			return (tfval == "true" || tfval == "yes" || tfval == "t" || tfval == "y");
		}
		return false;
	}

	ShipIDKey ReadShipIDKeyFromValue(Json::Value& val, const char* tag)
	{
		if (val.HasMember(tag))
		{
			if (val.IsUint64())
				return (ShipIDKey)val[tag].GetUint64();
			else if (val.IsUint())
				return val[tag].GetUint();
			else if (val.IsString())
				return (ShipIDKey)stoul(val[tag].GetString());
		}
		return false;
	}

	TimePoint ReadTimepointFromValue(Json::Value& val, const char* tag)
	{
		if (val.HasMember(tag))
		{
			string tmpval = val[tag].GetString();
			return TimePoint(tmpval);
		}
		else return TimePoint();
	}

	StaticType ReadStaticFromValues(Json::Value& val)
	{
		assert(val.IsObject());
		StaticType re;
		re.imo = ReadShipIDKeyFromValue(val, tag::IMO);
		re.mmsi = ReadShipIDKeyFromValue(val, tag::MMSI);
		re.Dim.L = ReadFromValue<Real>(val, tag::LOA);
		re.Dim.B = ReadFromValue<Real>(val, tag::Beam);
		re.Dim.T = ReadFromValue<Real>(val, tag::Draught);
		return re;
	}

	XY ReadXYFromValue(Json::Value& val)
	{
		XY re;
		re.longitude = ReadFromValue<Real>(val, tag::Lon);
		re.latitude = ReadFromValue<Real>(val, tag::Lat);
		return re;
	}

	XTD ReadXtdFromValue(Json::Value& val)
	{
		XTD re;
		re.xtdPortside = ReadFromValue<Real>(val, tag::XTD_P);
		re.xtdStarboard = ReadFromValue<Real>(val, tag::XTD_S);
		return re;
	}

	VVa ReadVelFromValue(Json::Value& val)
	{
		VVa re;
		re.speed = ReadFromValue<Real>(val, tag::SOG);
		re.angle = ReadFromValue<Real>(val, tag::COG);
		return re;
	}

	TimePoint ReadTimestampFromValue(Json::Value& val)
	{
		return ReadTimepointFromValue(val, tag::Timestamp);
	}

	XY& ReadTrajPointFromValue(Json::Value& val, XY& re)
	{
		assert(val.IsObject());
		re = ReadXYFromValue(val);
		return re;
	}

	XYXtd& ReadTrajPointFromValue(Json::Value& val, XYXtd& re)
	{
		assert(val.IsObject());
		re.Pos = ReadXYFromValue(val);
		re.Xtd = ReadXtdFromValue(val);
		return re;
	}

	XYT& ReadTrajPointFromValue(Json::Value& val, XYT& re)
	{
		assert(val.IsObject());
		re.Pos = ReadXYFromValue(val);
		re.TP = ReadTimestampFromValue(val);
		return re;
	}

	XYVVaT& ReadTrajPointFromValue(Json::Value& val, XYVVaT& re)
	{
		assert(val.IsObject());
		re.Pos = ReadXYFromValue(val);
		re.Vel = ReadVelFromValue(val);
		re.TP = ReadTimestampFromValue(val);
		return re;
	}

	XYVVaXtdT& ReadTrajPointFromValue(Json::Value& val, XYVVaXtdT& re)
	{
		assert(val.IsObject());
		re.Pos = ReadXYFromValue(val);
		re.Vel = ReadVelFromValue(val);
		re.Xtd = ReadXtdFromValue(val);
		re.TP = ReadTimestampFromValue(val);
		return re;
	}

	template <typename TList>
	TList ReadTrajectoryFromValues(Json::Value& valDynamic)
	{
		assert(valDynamic.IsArray());
		TList re;
		for (auto& wpVal : valDynamic.GetArray())
		{
			typename TList::value_type wp;
			re.push_back(ReadTrajPointFromValue(wpVal, wp));
		}
		return re;
	}
	
	template <>
	vector<XYList> ReadFelineJsonFile(const string filepath)
	{
		// Return value
		vector<XYList> re;
		// read file and parse json
		ifstream ifs(filepath);
		Json::IStreamWrapper isw(ifs);
		Json::Document doc;
		doc.ParseStream(isw);

		// read routes
		assert(doc.IsObject());
		assert(doc.HasMember(tag::Routes));
		Json::Value& routes = doc[tag::Routes];
		assert(routes.IsArray());

		// Read each routes
		for (auto& route : routes.GetArray()) {
			XYList tmpTraj;

			// No need to read static data

			// fill out dynamic data
			assert(route.HasMember(tag::Dynamic));
			tmpTraj = ReadTrajectoryFromValues<XYList>(route[tag::Dynamic]);

			// push to return value
			re.push_back(tmpTraj);
		}
		return re;
	}

	template <>
	vector<XYXtdList> ReadFelineJsonFile(const string filepath)
	{
		// Return value
		vector<XYXtdList> re;
		// read file and parse json
		ifstream ifs(filepath);
		Json::IStreamWrapper isw(ifs);
		Json::Document doc;
		doc.ParseStream(isw);

		// read routes
		assert(doc.IsObject());
		assert(doc.HasMember(tag::Routes));
		Json::Value& routes = doc[tag::Routes];
		assert(routes.IsArray());

		// Read each routes
		for (auto& route : routes.GetArray()) {
			XYXtdList tmpTraj;

			// No need to read static data

			// fill out dynamic data
			assert(route.HasMember(tag::Dynamic));
			tmpTraj = ReadTrajectoryFromValues<XYXtdList>(route[tag::Dynamic]);

			// push to return value
			re.push_back(tmpTraj);
		}
		return re;
	}

	template <>
	vector<XYTList> ReadFelineJsonFile(const string filepath)
	{
		// Return value
		vector<XYTList> re;
		// read file and parse json
		ifstream ifs(filepath);
		Json::IStreamWrapper isw(ifs);
		Json::Document doc;
		doc.ParseStream(isw);

		// read routes
		assert(doc.IsObject());
		assert(doc.HasMember(tag::Routes));
		Json::Value& routes = doc[tag::Routes];
		assert(routes.IsArray());

		// Read each routes
		for (auto& route : routes.GetArray()) {
			XYTList tmpTraj;

			// No need to read static data

			// fill out dynamic data
			assert(route.HasMember(tag::Dynamic));
			tmpTraj = ReadTrajectoryFromValues<XYTList>(route[tag::Dynamic]);

			// push to return value
			re.push_back(tmpTraj);
		}
		return re;
	}

	template <>
	vector<XYVVaTList> ReadFelineJsonFile(const string filepath)
	{
		// Return value
		vector<XYVVaTList> re;
		// read file and parse json
		ifstream ifs(filepath);
		Json::IStreamWrapper isw(ifs);
		Json::Document doc;
		doc.ParseStream(isw);

		// read routes
		assert(doc.IsObject());
		assert(doc.HasMember(tag::Routes));
		Json::Value& routes = doc[tag::Routes];
		assert(routes.IsArray());

		// Read each routes
		for (auto& route : routes.GetArray()) {
			XYVVaTList tmpTraj;

			// No need to read static data

			// fill out dynamic data
			assert(route.HasMember(tag::Dynamic));
			tmpTraj = ReadTrajectoryFromValues<XYVVaTList>(route[tag::Dynamic]);

			// push to return value
			re.push_back(tmpTraj);
		}
		return re;
	}

	template <>
	vector<XYVVaXtdTList> ReadFelineJsonFile(const string filepath)
	{
		// Return value
		vector<XYVVaXtdTList> re;
		// read file and parse json
		ifstream ifs(filepath);
		Json::IStreamWrapper isw(ifs);
		Json::Document doc;
		doc.ParseStream(isw);

		// read routes
		assert(doc.IsObject());
		assert(doc.HasMember(tag::Routes));
		Json::Value& routes = doc[tag::Routes];
		assert(routes.IsArray());

		// Read each routes
		for (auto& route : routes.GetArray()) {
			XYVVaXtdTList tmpTraj;

			// No need to read static data

			// fill out dynamic data
			assert(route.HasMember(tag::Dynamic));
			tmpTraj = ReadTrajectoryFromValues<XYVVaXtdTList>(route[tag::Dynamic]);

			// push to return value
			re.push_back(tmpTraj);
		}
		return re;
	}

	template <>
	vector<AisTrajectory<XYList>> ReadFelineJsonFile(const string filepath)
	{
		// Return value
		vector<AisTrajectory<XYList>> re;
		// read file and parse json
		ifstream ifs(filepath);
		Json::IStreamWrapper isw(ifs);
		Json::Document doc;
		doc.ParseStream(isw);

		// read routes
		assert(doc.IsObject());
		assert(doc.HasMember(tag::Routes));
		Json::Value& routes = doc[tag::Routes];
		assert(routes.IsArray());

		// Read each routes
		for (auto& route : routes.GetArray()) {
			AisTrajectory<XYList> tmpTraj;

			// fill out static data
			assert(route.HasMember(tag::Static));
			tmpTraj.staticInfo = ReadStaticFromValues(route[tag::Static]);

			// fill out dynamic data
			assert(route.HasMember(tag::Dynamic));
			tmpTraj.trajectory = ReadTrajectoryFromValues<XYList>(route[tag::Dynamic]);

			// push to return value
			re.push_back(tmpTraj);
		}
		return re;
	}

	template <>
	vector<AisTrajectory<XYXtdList>> ReadFelineJsonFile(const string filepath)
	{
		// Return value
		vector<AisTrajectory<XYXtdList>> re;
		// read file and parse json
		ifstream ifs(filepath);
		Json::IStreamWrapper isw(ifs);
		Json::Document doc;
		doc.ParseStream(isw);

		// read routes
		assert(doc.IsObject());
		assert(doc.HasMember(tag::Routes));
		Json::Value& routes = doc[tag::Routes];
		assert(routes.IsArray());

		// Read each routes
		for (auto& route : routes.GetArray()) {
			AisTrajectory<XYXtdList> tmpTraj;

			// fill out static data
			assert(route.HasMember(tag::Static));
			tmpTraj.staticInfo = ReadStaticFromValues(route[tag::Static]);

			// fill out dynamic data
			assert(route.HasMember(tag::Dynamic));
			tmpTraj.trajectory = ReadTrajectoryFromValues<XYXtdList>(route[tag::Dynamic]);

			// push to return value
			re.push_back(tmpTraj);
		}
		return re;
	}

	template <>
	vector<AisTrajectory<XYTList>> ReadFelineJsonFile(const string filepath)
	{
		// Return value
		vector<AisTrajectory<XYTList>> re;
		// read file and parse json
		ifstream ifs(filepath);
		Json::IStreamWrapper isw(ifs);
		Json::Document doc;
		doc.ParseStream(isw);

		// read routes
		assert(doc.IsObject());
		assert(doc.HasMember(tag::Routes));
		Json::Value& routes = doc[tag::Routes];
		assert(routes.IsArray());

		// Read each routes
		for (auto& route : routes.GetArray()) {
			AisTrajectory<XYTList> tmpTraj;

			// fill out static data
			assert(route.HasMember(tag::Static));
			tmpTraj.staticInfo = ReadStaticFromValues(route[tag::Static]);

			// fill out dynamic data
			assert(route.HasMember(tag::Dynamic));
			tmpTraj.trajectory = ReadTrajectoryFromValues<XYTList>(route[tag::Dynamic]);

			// push to return value
			re.push_back(tmpTraj);
		}
		return re;
	}

	template <>
	vector<AisTrajectory<XYVVaTList>> ReadFelineJsonFile(const string filepath)
	{
		// Return value
		vector<AisTrajectory<XYVVaTList>> re;
		// read file and parse json
		ifstream ifs(filepath);
		Json::IStreamWrapper isw(ifs);
		Json::Document doc;
		doc.ParseStream(isw);

		// read routes
		assert(doc.IsObject());
		assert(doc.HasMember(tag::Routes));
		Json::Value& routes = doc[tag::Routes];
		assert(routes.IsArray());

		// Read each routes
		for (auto& route : routes.GetArray()) {
			AisTrajectory<XYVVaTList> tmpTraj;

			// fill out static data
			assert(route.HasMember(tag::Static));
			tmpTraj.staticInfo = ReadStaticFromValues(route[tag::Static]);

			// fill out dynamic data
			assert(route.HasMember(tag::Dynamic));
			tmpTraj.trajectory = ReadTrajectoryFromValues<XYVVaTList>(route[tag::Dynamic]);

			// push to return value
			re.push_back(tmpTraj);
		}
		return re;
	}

	template <>
	vector<AisTrajectory<XYVVaXtdTList>> ReadFelineJsonFile(const string filepath)
	{
		// Return value
		vector<AisTrajectory<XYVVaXtdTList>> re;
		// read file and parse json
		ifstream ifs(filepath);
		Json::IStreamWrapper isw(ifs);
		Json::Document doc;
		doc.ParseStream(isw);

		// read routes
		assert(doc.IsObject());
		assert(doc.HasMember(tag::Routes));
		Json::Value& routes = doc[tag::Routes];
		assert(routes.IsArray());

		// Read each routes
		for (auto& route : routes.GetArray()) {
			AisTrajectory<XYVVaXtdTList> tmpTraj;

			// fill out static data
			assert(route.HasMember(tag::Static));
			tmpTraj.staticInfo = ReadStaticFromValues(route[tag::Static]);

			// fill out dynamic data
			assert(route.HasMember(tag::Dynamic));
			tmpTraj.trajectory = ReadTrajectoryFromValues<XYVVaXtdTList>(route[tag::Dynamic]);

			// push to return value
			re.push_back(tmpTraj);
		}
		return re;
	}
}

// Write Feline Json files
namespace HashColon::Feline::IO
{
	Json::Value& WriteStaticToValue(const StaticType& staticData, Json::Value& val, Json::MemoryPoolAllocator<>& alloc)
	{
		assert(val.IsObject());
		val.AddMember(Json::Value(tag::IMO, (unsigned int)strlen(tag::IMO), alloc), Json::Value(staticData.imo), alloc);
		val.AddMember(Json::Value(tag::MMSI, (unsigned int)strlen(tag::MMSI), alloc), Json::Value(staticData.mmsi), alloc);
		val.AddMember(Json::Value(tag::LOA, (unsigned int)strlen(tag::LOA), alloc), Json::Value(staticData.Dim.L), alloc);
		val.AddMember(Json::Value(tag::Beam, (unsigned int)strlen(tag::Beam), alloc), Json::Value(staticData.Dim.B), alloc);
		val.AddMember(Json::Value(tag::Draught, (unsigned int)strlen(tag::Draught), alloc), Json::Value(staticData.Dim.T), alloc);
		return val;
	}

	Json::Value& WritePosToValue(const Position& pos, Json::Value& val, Json::MemoryPoolAllocator<>& alloc)
	{
		val.AddMember(Json::Value(tag::Lon, (unsigned int)strlen(tag::Lon), alloc), Json::Value(pos.longitude), alloc);
		val.AddMember(Json::Value(tag::Lon, (unsigned int)strlen(tag::Lon), alloc), Json::Value(pos.longitude), alloc);
		return val;
	}

	Json::Value& WriteXtdToValue(const XTD& xtd, Json::Value& val, Json::MemoryPoolAllocator<>& alloc)
	{
		val.AddMember(Json::Value(tag::XTD_P, (unsigned int)strlen(tag::XTD_P), alloc), Json::Value(xtd.xtdPortside), alloc);
		val.AddMember(Json::Value(tag::XTD_S, (unsigned int)strlen(tag::XTD_S), alloc), Json::Value(xtd.xtdStarboard), alloc);
		return val;
	}

	Json::Value& WriteVelToValue(const VVa& vel, Json::Value& val, Json::MemoryPoolAllocator<>& alloc)
	{
		val.AddMember(Json::Value(tag::SOG, (unsigned int)strlen(tag::SOG), alloc), Json::Value(vel.speed), alloc);
		val.AddMember(Json::Value(tag::COG, (unsigned int)strlen(tag::COG), alloc), Json::Value(vel.angle), alloc);
		return val;
	}

	Json::Value& WriteTimestampToValue(const TimePoint& tp, Json::Value& val, Json::MemoryPoolAllocator<>& alloc)
	{
		string tpstr = tp.toString();
		val.AddMember(Json::Value(tag::Timestamp, (unsigned int)strlen(tag::Timestamp), alloc),
			Json::Value(tpstr.c_str(), (unsigned int)tpstr.size(), alloc), alloc);
		return val;
	}

	Json::Value& WriteTrajPointToValue(const XY& wp, Json::Value& val, Json::MemoryPoolAllocator<>& alloc)
	{
		WritePosToValue(wp, val, alloc);
		return val;
	}

	Json::Value& WriteTrajPointToValue(const XYXtd& wp, Json::Value& val, Json::MemoryPoolAllocator<>& alloc)
	{
		WritePosToValue(wp.Pos, val, alloc);
		WriteXtdToValue(wp.Xtd, val, alloc);
		return val;
	}

	Json::Value& WriteTrajPointToValue(const XYT& wp, Json::Value& val, Json::MemoryPoolAllocator<>& alloc)
	{
		WritePosToValue(wp.Pos, val, alloc);
		WriteTimestampToValue(wp.TP, val, alloc);
		return val;
	}

	Json::Value& WriteTrajPointToValue(const XYVVaT& wp, Json::Value& val, Json::MemoryPoolAllocator<>& alloc)
	{
		WritePosToValue(wp.Pos, val, alloc);
		WriteVelToValue(wp.Vel, val, alloc);
		WriteTimestampToValue(wp.TP, val, alloc);
		return val;
	}

	Json::Value& WriteTrajPointToValue(const XYVVaXtdT& wp, Json::Value& val, Json::MemoryPoolAllocator<>& alloc)
	{
		WritePosToValue(wp.Pos, val, alloc);
		WriteVelToValue(wp.Vel, val, alloc);
		WriteXtdToValue(wp.Xtd, val, alloc);
		WriteTimestampToValue(wp.TP, val, alloc);
		return val;
	}

	template <typename TList>
	Json::Value& WriteTrajectoryToValues(const TList& traj, Json::Value& val, Json::MemoryPoolAllocator<>& alloc)
	{
		assert(val.IsArray());
		for (const auto& wp : traj)
		{
			Json::Value wpJson(Json::kObjectType);
			val.PushBack(WriteTrajPointToValue(wp, wpJson, alloc), alloc);
		}
		return val;
	}

	template <>
	Json::Document WriteFelineJsonFile(
		const string outputFilepath, const vector<XYList>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"routes" : []
		}
		)str");

		for (const auto& route : routes)
		{
			Json::Value routeJson(Json::kObjectType);

			// set static & dynamic
			routeJson.AddMember("static", Json::Value(Json::kObjectType), alloc);
			Json::Value& dynamicVal = routeJson.AddMember("dynamic", Json::Value(Json::kArrayType), alloc);

			// no static data is given, ignore lol

			// write dynamic data
			WriteTrajectoryToValues(route, dynamicVal, alloc);

			// push to routes
			doc["routes"].PushBack(routeJson, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteFelineJsonFile(
		const string outputFilepath, const vector<XYXtdList>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"routes" : []
		}
		)str");

		for (const auto& route : routes)
		{
			Json::Value routeJson(Json::kObjectType);

			// set static & dynamic
			routeJson.AddMember("static", Json::Value(Json::kObjectType), alloc);
			Json::Value& dynamicVal = routeJson.AddMember("dynamic", Json::Value(Json::kArrayType), alloc);

			// no static data is given, ignore lol

			// write dynamic data
			WriteTrajectoryToValues(route, dynamicVal, alloc);

			// push to routes
			doc["routes"].PushBack(routeJson, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteFelineJsonFile(
		const string outputFilepath, const vector<XYTList>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"routes" : []
		}
		)str");

		for (const auto& route : routes)
		{
			Json::Value routeJson(Json::kObjectType);

			// set static & dynamic
			routeJson.AddMember("static", Json::Value(Json::kObjectType), alloc);
			Json::Value& dynamicVal = routeJson.AddMember("dynamic", Json::Value(Json::kArrayType), alloc);

			// no static data is given, ignore lol

			// write dynamic data
			WriteTrajectoryToValues(route, dynamicVal, alloc);

			// push to routes
			doc["routes"].PushBack(routeJson, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteFelineJsonFile(
		const string outputFilepath, const vector<XYVVaTList>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"routes" : []
		}
		)str");

		for (const auto& route : routes)
		{
			Json::Value routeJson(Json::kObjectType);

			// set static & dynamic
			routeJson.AddMember("static", Json::Value(Json::kObjectType), alloc);
			Json::Value& dynamicVal = routeJson.AddMember("dynamic", Json::Value(Json::kArrayType), alloc);

			// no static data is given, ignore lol

			// write dynamic data
			WriteTrajectoryToValues(route, dynamicVal, alloc);

			// push to routes
			doc["routes"].PushBack(routeJson, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteFelineJsonFile(
		const string outputFilepath, const vector<XYVVaXtdTList>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"routes" : []
		}
		)str");

		for (const auto& route : routes)
		{
			Json::Value routeJson(Json::kObjectType);

			// set static & dynamic
			routeJson.AddMember("static", Json::Value(Json::kObjectType), alloc);
			Json::Value& dynamicVal = routeJson.AddMember("dynamic", Json::Value(Json::kArrayType), alloc);

			// no static data is given, ignore lol

			// write dynamic data
			WriteTrajectoryToValues(route, dynamicVal, alloc);

			// push to routes
			doc["routes"].PushBack(routeJson, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteFelineJsonFile(
		const string outputFilepath, const vector<AisTrajectory<XYList>>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"routes" : []
		}
		)str");

		for (const auto& route : routes)
		{
			Json::Value routeJson(Json::kObjectType);

			// set static & dynamic
			Json::Value& staticVal = routeJson.AddMember("static", Json::Value(Json::kObjectType), alloc);
			Json::Value& dynamicVal = routeJson.AddMember("dynamic", Json::Value(Json::kArrayType), alloc);

			// write static data			
			WriteStaticToValue(route.staticInfo, staticVal, alloc);

			// write dynamic data
			WriteTrajectoryToValues(route.trajectory, dynamicVal, alloc);

			// push to routes
			doc["routes"].PushBack(routeJson, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteFelineJsonFile(
		const string outputFilepath, const vector<AisTrajectory<XYXtdList>>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"routes" : []
		}
		)str");

		for (const auto& route : routes)
		{
			Json::Value routeJson(Json::kObjectType);

			// set static & dynamic
			Json::Value& staticVal = routeJson.AddMember("static", Json::Value(Json::kObjectType), alloc);
			Json::Value& dynamicVal = routeJson.AddMember("dynamic", Json::Value(Json::kArrayType), alloc);

			// write static data			
			WriteStaticToValue(route.staticInfo, staticVal, alloc);

			// write dynamic data
			WriteTrajectoryToValues(route.trajectory, dynamicVal, alloc);

			// push to routes
			doc["routes"].PushBack(routeJson, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteFelineJsonFile(
		const string outputFilepath, const vector<AisTrajectory<XYTList>>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"routes" : []
		}
		)str");

		for (const auto& route : routes)
		{
			Json::Value routeJson(Json::kObjectType);

			// set static & dynamic
			Json::Value& staticVal = routeJson.AddMember("static", Json::Value(Json::kObjectType), alloc);
			Json::Value& dynamicVal = routeJson.AddMember("dynamic", Json::Value(Json::kArrayType), alloc);

			// write static data			
			WriteStaticToValue(route.staticInfo, staticVal, alloc);

			// write dynamic data
			WriteTrajectoryToValues(route.trajectory, dynamicVal, alloc);

			// push to routes
			doc["routes"].PushBack(routeJson, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteFelineJsonFile(
		const string outputFilepath, const vector<AisTrajectory<XYVVaTList>>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"routes" : []
		}
		)str");

		for (const auto& route : routes)
		{
			Json::Value routeJson(Json::kObjectType);

			// set static & dynamic
			Json::Value& staticVal = routeJson.AddMember("static", Json::Value(Json::kObjectType), alloc);
			Json::Value& dynamicVal = routeJson.AddMember("dynamic", Json::Value(Json::kArrayType), alloc);

			// write static data			
			WriteStaticToValue(route.staticInfo, staticVal, alloc);

			// write dynamic data
			WriteTrajectoryToValues(route.trajectory, dynamicVal, alloc);

			// push to routes
			doc["routes"].PushBack(routeJson, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteFelineJsonFile(
		const string outputFilepath, const vector<AisTrajectory<XYVVaXtdTList>>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"routes" : []
		}
		)str");

		for (const auto& route : routes)
		{
			Json::Value routeJson(Json::kObjectType);

			// set static & dynamic
			Json::Value& staticVal = routeJson.AddMember("static", Json::Value(Json::kObjectType), alloc);
			Json::Value& dynamicVal = routeJson.AddMember("dynamic", Json::Value(Json::kArrayType), alloc);

			// write static data			
			WriteStaticToValue(route.staticInfo, staticVal, alloc);

			// write dynamic data
			WriteTrajectoryToValues(route.trajectory, dynamicVal, alloc);

			// push to routes
			doc["routes"].PushBack(routeJson, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}
}

// Write GeoJson files
namespace HashColon::Feline::IO
{
	Json::Value& WriteTrajPointToGeoJsonCoord(const XYList& traj, Json::Value& val, Json::MemoryPoolAllocator<>& alloc)
	{
		assert(val.IsArray());
		for (const auto& wp : traj)
		{
			Json::Value wpJson(Json::kArrayType);
			wpJson.PushBack(Json::Value(wp.longitude), alloc);
			wpJson.PushBack(Json::Value(wp.latitude), alloc);
			val.PushBack(wpJson, alloc);
		}
		return val;
	}

	template <>
	Json::Document WriteGeoJsonFile(
		const string outputFilepath, const vector<AisTrajectory<XYList>>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"type": "FeatureCollection",
			"features" : []
		}
		)str");

		for (const AisTrajectory<XYList>& route : routes)
		{
			// make base structure for geojson "Feature"
			Json::Value feature;
			feature.SetObject();
			feature.AddMember("type", Json::Value("Feature"), alloc);
			feature.AddMember("properties", Json::Value(Json::kObjectType), alloc);
			feature["properties"].AddMember("static", Json::Value(Json::kArrayType), alloc);
			feature["properties"].AddMember("dynamic", Json::Value(Json::kArrayType), alloc);
			feature.AddMember("geometry", Json::Value(Json::kObjectType), alloc);
			feature["geometry"].AddMember("type", Json::Value("LineString"), alloc);
			feature["geometry"].AddMember("coordinates", Json::Value(Json::kArrayType), alloc);

			// add static/dynamic tags
			WriteStaticToValue(route.staticInfo, feature["properties"]["static"], alloc);
			WriteTrajectoryToValues(route.trajectory, feature["properties"]["dynamic"], alloc);

			// add coordinates
			XYList xylist = route.trajectory;
			WriteTrajPointToGeoJsonCoord(xylist, feature["geometry"]["coordinates"], alloc);

			doc["features"].PushBack(feature, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteGeoJsonFile(
		const string outputFilepath, const vector<AisTrajectory<XYXtdList>>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"type": "FeatureCollection",
			"features" : []
		}
		)str");

		for (const AisTrajectory<XYXtdList>& route : routes)
		{
			// make base structure for geojson "Feature"
			Json::Value feature;
			feature.SetObject();
			feature.AddMember("type", Json::Value("Feature"), alloc);
			feature.AddMember("properties", Json::Value(Json::kObjectType), alloc);
			feature["properties"].AddMember("static", Json::Value(Json::kArrayType), alloc);
			feature["properties"].AddMember("dynamic", Json::Value(Json::kArrayType), alloc);
			feature.AddMember("geometry", Json::Value(Json::kObjectType), alloc);
			feature["geometry"].AddMember("type", Json::Value("LineString"), alloc);
			feature["geometry"].AddMember("coordinates", Json::Value(Json::kArrayType), alloc);

			// add static/dynamic tags
			WriteStaticToValue(route.staticInfo, feature["properties"]["static"], alloc);
			WriteTrajectoryToValues(route.trajectory, feature["properties"]["dynamic"], alloc);

			// add coordinates
			XYList xylist = route.trajectory;
			WriteTrajPointToGeoJsonCoord(xylist, feature["geometry"]["coordinates"], alloc);

			doc["features"].PushBack(feature, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteGeoJsonFile(
		const string outputFilepath, const vector<AisTrajectory<XYTList>>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"type": "FeatureCollection",
			"features" : []
		}
		)str");

		for (const AisTrajectory<XYTList>& route : routes)
		{
			// make base structure for geojson "Feature"
			Json::Value feature;
			feature.SetObject();
			feature.AddMember("type", Json::Value("Feature"), alloc);
			feature.AddMember("properties", Json::Value(Json::kObjectType), alloc);
			feature["properties"].AddMember("static", Json::Value(Json::kArrayType), alloc);
			feature["properties"].AddMember("dynamic", Json::Value(Json::kArrayType), alloc);
			feature.AddMember("geometry", Json::Value(Json::kObjectType), alloc);
			feature["geometry"].AddMember("type", Json::Value("LineString"), alloc);
			feature["geometry"].AddMember("coordinates", Json::Value(Json::kArrayType), alloc);

			// add static/dynamic tags
			WriteStaticToValue(route.staticInfo, feature["properties"]["static"], alloc);
			WriteTrajectoryToValues(route.trajectory, feature["properties"]["dynamic"], alloc);

			// add coordinates
			XYList xylist = route.trajectory;
			WriteTrajPointToGeoJsonCoord(xylist, feature["geometry"]["coordinates"], alloc);

			doc["features"].PushBack(feature, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteGeoJsonFile(
		const string outputFilepath, const vector<AisTrajectory<XYVVaTList>>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"type": "FeatureCollection",
			"features" : []
		}
		)str");

		for (const AisTrajectory<XYVVaTList>& route : routes)
		{
			// make base structure for geojson "Feature"
			Json::Value feature;
			feature.SetObject();
			feature.AddMember("type", Json::Value("Feature"), alloc);
			feature.AddMember("properties", Json::Value(Json::kObjectType), alloc);
			feature["properties"].AddMember("static", Json::Value(Json::kArrayType), alloc);
			feature["properties"].AddMember("dynamic", Json::Value(Json::kArrayType), alloc);
			feature.AddMember("geometry", Json::Value(Json::kObjectType), alloc);
			feature["geometry"].AddMember("type", Json::Value("LineString"), alloc);
			feature["geometry"].AddMember("coordinates", Json::Value(Json::kArrayType), alloc);

			// add static/dynamic tags
			WriteStaticToValue(route.staticInfo, feature["properties"]["static"], alloc);
			WriteTrajectoryToValues(route.trajectory, feature["properties"]["dynamic"], alloc);

			// add coordinates
			XYList xylist = route.trajectory;
			WriteTrajPointToGeoJsonCoord(xylist, feature["geometry"]["coordinates"], alloc);

			doc["features"].PushBack(feature, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteGeoJsonFile(
		const string outputFilepath, const vector<AisTrajectory<XYVVaXtdTList>>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"type": "FeatureCollection",
			"features" : []
		}
		)str");

		for (const AisTrajectory<XYVVaXtdTList>& route : routes)
		{
			// make base structure for geojson "Feature"
			Json::Value feature;
			feature.SetObject();
			feature.AddMember("type", Json::Value("Feature"), alloc);
			feature.AddMember("properties", Json::Value(Json::kObjectType), alloc);
			feature["properties"].AddMember("static", Json::Value(Json::kArrayType), alloc);
			feature["properties"].AddMember("dynamic", Json::Value(Json::kArrayType), alloc);
			feature.AddMember("geometry", Json::Value(Json::kObjectType), alloc);
			feature["geometry"].AddMember("type", Json::Value("LineString"), alloc);
			feature["geometry"].AddMember("coordinates", Json::Value(Json::kArrayType), alloc);

			// add static/dynamic tags
			WriteStaticToValue(route.staticInfo, feature["properties"]["static"], alloc);
			WriteTrajectoryToValues(route.trajectory, feature["properties"]["dynamic"], alloc);

			// add coordinates
			XYList xylist = route.trajectory;
			WriteTrajPointToGeoJsonCoord(xylist, feature["geometry"]["coordinates"], alloc);

			doc["features"].PushBack(feature, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteGeoJsonFile(
		const string outputFilepath, const vector<XYList>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"type": "FeatureCollection",
			"features" : []
		}
		)str");

		for (const XYList& route : routes)
		{
			// make base structure for geojson "Feature"
			Json::Value feature;
			feature.SetObject();
			feature.AddMember("type", Json::Value("Feature"), alloc);
			feature.AddMember("properties", Json::Value(Json::kObjectType), alloc);
			feature["properties"].AddMember("dynamic", Json::Value(Json::kArrayType), alloc);
			feature.AddMember("geometry", Json::Value(Json::kObjectType), alloc);
			feature["geometry"].AddMember("type", Json::Value("LineString"), alloc);
			feature["geometry"].AddMember("coordinates", Json::Value(Json::kArrayType), alloc);

			// add tags
			WriteTrajectoryToValues(route, feature["properties"]["dynamic"], alloc);

			// add coordinates
			XYList xylist = route;
			WriteTrajPointToGeoJsonCoord(xylist, feature["geometry"]["coordinates"], alloc);

			doc["features"].PushBack(feature, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteGeoJsonFile(
		const string outputFilepath, const vector<XYXtdList>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"type": "FeatureCollection",
			"features" : []
		}
		)str");

		for (const XYXtdList& route : routes)
		{
			// make base structure for geojson "Feature"
			Json::Value feature;
			feature.SetObject();
			feature.AddMember("type", Json::Value("Feature"), alloc);
			feature.AddMember("properties", Json::Value(Json::kObjectType), alloc);
			feature["properties"].AddMember("dynamic", Json::Value(Json::kArrayType), alloc);
			feature.AddMember("geometry", Json::Value(Json::kObjectType), alloc);
			feature["geometry"].AddMember("type", Json::Value("LineString"), alloc);
			feature["geometry"].AddMember("coordinates", Json::Value(Json::kArrayType), alloc);

			// add tags
			WriteTrajectoryToValues(route, feature["properties"]["dynamic"], alloc);

			// add coordinates
			XYList xylist = route;
			WriteTrajPointToGeoJsonCoord(xylist, feature["geometry"]["coordinates"], alloc);

			doc["features"].PushBack(feature, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteGeoJsonFile(
		const string outputFilepath, const vector<XYTList>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"type": "FeatureCollection",
			"features" : []
		}
		)str");

		for (const XYTList& route : routes)
		{
			// make base structure for geojson "Feature"
			Json::Value feature;
			feature.SetObject();
			feature.AddMember("type", Json::Value("Feature"), alloc);
			feature.AddMember("properties", Json::Value(Json::kObjectType), alloc);
			feature["properties"].AddMember("dynamic", Json::Value(Json::kArrayType), alloc);
			feature.AddMember("geometry", Json::Value(Json::kObjectType), alloc);
			feature["geometry"].AddMember("type", Json::Value("LineString"), alloc);
			feature["geometry"].AddMember("coordinates", Json::Value(Json::kArrayType), alloc);

			// add tags
			WriteTrajectoryToValues(route, feature["properties"]["dynamic"], alloc);

			// add coordinates
			XYList xylist = route;
			WriteTrajPointToGeoJsonCoord(xylist, feature["geometry"]["coordinates"], alloc);

			doc["features"].PushBack(feature, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteGeoJsonFile(
		const string outputFilepath, const vector<XYVVaTList>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"type": "FeatureCollection",
			"features" : []
		}
		)str");

		for (const XYVVaTList& route : routes)
		{
			// make base structure for geojson "Feature"
			Json::Value feature;
			feature.SetObject();
			feature.AddMember("type", Json::Value("Feature"), alloc);
			feature.AddMember("properties", Json::Value(Json::kObjectType), alloc);
			feature["properties"].AddMember("dynamic", Json::Value(Json::kArrayType), alloc);
			feature.AddMember("geometry", Json::Value(Json::kObjectType), alloc);
			feature["geometry"].AddMember("type", Json::Value("LineString"), alloc);
			feature["geometry"].AddMember("coordinates", Json::Value(Json::kArrayType), alloc);

			// add tags
			WriteTrajectoryToValues(route, feature["properties"]["dynamic"], alloc);

			// add coordinates
			XYList xylist = route;
			WriteTrajPointToGeoJsonCoord(xylist, feature["geometry"]["coordinates"], alloc);

			doc["features"].PushBack(feature, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}

	template <>
	Json::Document WriteGeoJsonFile(
		const string outputFilepath, const vector<XYVVaXtdTList>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		Json::Document doc;

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"type": "FeatureCollection",
			"features" : []
		}
		)str");

		for (const XYVVaXtdTList& route : routes)
		{
			// make base structure for geojson "Feature"
			Json::Value feature;
			feature.SetObject();
			feature.AddMember("type", Json::Value("Feature"), alloc);
			feature.AddMember("properties", Json::Value(Json::kObjectType), alloc);
			feature["properties"].AddMember("dynamic", Json::Value(Json::kArrayType), alloc);
			feature.AddMember("geometry", Json::Value(Json::kObjectType), alloc);
			feature["geometry"].AddMember("type", Json::Value("LineString"), alloc);
			feature["geometry"].AddMember("coordinates", Json::Value(Json::kArrayType), alloc);

			// add tags
			WriteTrajectoryToValues(route, feature["properties"]["dynamic"], alloc);

			// add coordinates
			XYList xylist = route;
			WriteTrajPointToGeoJsonCoord(xylist, feature["geometry"]["coordinates"], alloc);

			doc["features"].PushBack(feature, alloc);
		}

		// print to file (use pretty writer if the writePretty option is on)
		ofstream ofs(outputFilepath);
		Json::OStreamWrapper osw(ofs);
		if (writePretty)
		{
			Json::PrettyWriter<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		else
		{
			Json::Writer<Json::OStreamWrapper> writer(osw);
			doc.Accept(writer);
		}
		return doc;
	}
}
