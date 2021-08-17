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

namespace _local
{
	namespace tag
	{
		const vector<const char*> lon = { "longitude", "lon", "lng" };
		const vector<const char*> lat = { "latitude", "lat" };
		const vector<const char*> xtd = { "xtd" };
		const vector<const char*> xtd_p = { "xtd_p" };
		const vector<const char*> xtd_s = { "xtd_s" };
	}	

	bool HasLon(Json::Value& v) 
	{
		bool re = false;
		for (auto t : tag::lon) { re = re || v.HasMember(t); }
		return re;
	}

	bool HasLat(Json::Value& v)
	{
		bool re = false;
		for (auto t : tag::lat) { re = re || v.HasMember(t); }
		return re;
	}

	bool HasXtd(Json::Value& v)
	{
		bool re = false;
		for (auto t : tag::xtd) { re = re || v.HasMember(t); }
		return re;
	}

	bool HasXtd_ps(Json::Value& v)
	{
		bool re_p = false;
		bool re_s = false;		
		for (auto t : tag::xtd_p) { re_p = re_p || v.HasMember(t); }
		for (auto t : tag::xtd_s) { re_s = re_s || v.HasMember(t); }
		return re_p && re_s;
	}


	void SetLon2XY(Json::Value& v, XY& p)
	{
		assert(HasLon(v));

		for (const char* tlon : tag::lon)
			if (v.HasMember(tlon))
			{
				assert((v[tlon].IsDouble()) || (v[tlon].IsString()));
				if (v[tlon].IsDouble())
					p.longitude = v[tlon].GetDouble();
				else if (v[tlon].IsString())
					p.longitude = stod(v[tlon].GetString());
				return;
			}
	}

	void SetLat2XY(Json::Value& v, XY& p)
	{
		assert(HasLat(v));

		for (const char* tlat : tag::lat)
			if (v.HasMember(tlat))
			{
				assert((v[tlat].IsDouble()) || (v[tlat].IsString()));
				if (v[tlat].IsDouble())
					p.latitude = v[tlat].GetDouble();
				else if (v[tlat].IsString())
					p.latitude = stod(v[tlat].GetString());
				return;
			}
	}

	void SetXtd2XYXtd(Json::Value& v, XYXtd& p)
	{
		bool hasxtd = HasXtd(v);
		bool hasxtdps = HasXtd_ps(v);
		assert(hasxtd || hasxtdps);

		if (hasxtd)
		{
			for (const char* txtd : tag::xtd)
				if (v.HasMember(txtd))
				{
					assert(v[txtd].IsArray());
					const Json::Value& xtdv = v[txtd].GetArray();

					if (xtdv[0].IsDouble())
						p.Xtd.xtdPortside = xtdv[0].GetDouble();
					else if (xtdv[0].IsString())
						p.Xtd.xtdPortside = stod(xtdv[0].GetString());

					if (xtdv[1].IsDouble())
						p.Xtd.xtdStarboard = xtdv[1].GetDouble();
					else if (xtdv[1].IsString())
						p.Xtd.xtdStarboard = stod(xtdv[1].GetString());

					return;
				}
		}
		else if (hasxtdps)
		{
			for (const char* txtd : tag::xtd_p)
				if (v.HasMember(txtd))
				{
					assert(v[txtd].IsDouble() || v[txtd].IsString());
					if (v[txtd].IsDouble())
						p.Xtd.xtdPortside = v[txtd].GetDouble();
					else if (v.IsString())
						p.Xtd.xtdPortside = stod(v[txtd].GetString());
					break;
				}
			for (const char* txtd : tag::xtd_s)
				if (v.HasMember(txtd))
				{
					assert(v[txtd].IsDouble() || v[txtd].IsString());
					if (v[txtd].IsDouble())
						p.Xtd.xtdStarboard = v[txtd].GetDouble();
					else if (v[txtd].IsString())
						p.Xtd.xtdStarboard = stod(v[txtd].GetString());
					break;
				}
		}		
	}
}

namespace HashColon::Feline::IO
{
	template <>
	vector<XYList> ReadFelineJsonFile<XYList>(string filepath)
	{
		// set logger 
		//GlobalLogger logger;
		//logger.Log({ { Tag::lvl, 5 } }) << "ReadJsonFile_SimpleXYList: Reading " << filepath << endl;

		vector<XYList> re;
		ifstream ifs(filepath);
		Json::IStreamWrapper isw(ifs);

		// parse JSON into rapidjson object
		Json::Document doc;
		doc.ParseStream(isw);

		// assertions
		assert(doc.IsObject());
		assert(doc.HasMember("routes"));
		Json::Value& routes = doc["routes"];
		assert(routes.IsArray());

		for (auto& route : routes.GetArray()) {
			assert(route.HasMember("dynamic"));
			assert(route["dynamic"].IsArray());

			XYList reRoute;
			for (auto& pt : route["dynamic"].GetArray())
			{				
				XY reXY;

				_local::SetLon2XY(pt, reXY);
				_local::SetLat2XY(pt, reXY);
				
				reRoute.push_back(reXY);
			}
			re.push_back(reRoute);
		}

		//logger.Log({ { Tag::lvl, 5 } }) << "ReadJsonFile_SimpleXYList: Finished reading " << filepath << endl;
		return re;
	}

	template <>
	vector<XYXtdList> ReadFelineJsonFile<XYXtdList>(string filepath)
	{		
		vector<XYXtdList> re;
		ifstream ifs(filepath);
		Json::IStreamWrapper isw(ifs);

		Json::Document doc;
		doc.ParseStream(isw);

		assert(doc.IsObject());
		assert(doc.HasMember("routes"));
		Json::Value& routes = doc["routes"];
		assert(routes.IsArray());

		for (auto& route : routes.GetArray()) {
			assert(route.HasMember("dynamic"));
			assert(route["dynamic"].IsArray());

			XYXtdList reRoute;
			for (auto& pt : route["dynamic"].GetArray())
			{	
				XY reXY;
				XYXtd reXYXtd;

				_local::SetLon2XY(pt, reXY);
				_local::SetLat2XY(pt, reXY);
				_local::SetXtd2XYXtd(pt, reXYXtd);

				reXYXtd.Pos = reXY;

				reRoute.push_back(reXYXtd);
			}
			re.push_back(reRoute);
		}

		//logger.Log({ { Tag::lvl, 5 } }) << "ReadJsonFile_SimpleXYList: Finished reading " << filepath << endl;
		return re;
	}

	template <>
	vector<AisTrajectory<>> ReadFelineJsonFile<AisTrajectory<>>(string filepath)
	{
		vector<AisTrajectory<>> re;
		ifstream ifs(filepath);
		Json::IStreamWrapper isw(ifs);

		Json::Document doc;
		doc.ParseStream(isw);

		assert(doc.IsObject());
		assert(doc.HasMember("routes"));
		Json::Value& routes = doc["routes"];		
		assert(routes.IsArray());

		for (auto& route : routes.GetArray()) {
			AisTrajectory tmpTraj;

			// fill out static data
			assert(route.HasMember("static"));
			assert(route["static"].IsObject());
			{
				assert(route["static"].HasMember("imo"));
				tmpTraj.staticInfo.imo = route["static"]["imo"].GetInt();
				assert(route["static"].HasMember("mmsi"));
				tmpTraj.staticInfo.mmsi = route["static"]["mmsi"].GetInt();
				assert(route["static"].HasMember("loa"));
				tmpTraj.staticInfo.Dim.L = route["static"]["loa"].GetDouble();
				assert(route["static"].HasMember("beam"));
				tmpTraj.staticInfo.Dim.B = route["static"]["beam"].GetDouble();
				assert(route["static"].HasMember("draught"));
				tmpTraj.staticInfo.Dim.T = route["static"]["draught"].GetDouble();
			}
			
			// fill out dynamic data
			assert(route.HasMember("dynamic"));
			assert(route["dynamic"].IsArray());			
			for (auto& wp : route["dynamic"].GetArray())
			{
				assert(wp.IsObject());
				XYVVaT tmpWP;
				if (wp.HasMember("latitude"))
					tmpWP.Pos.latitude = wp["latitude"].GetDouble();
				if (wp.HasMember("longitude"))
					tmpWP.Pos.longitude = wp["longiitude"].GetDouble();
				if (wp.HasMember("sog"))
					tmpWP.Vel.speed = wp["sog"].GetDouble();
				if (wp.HasMember("cog"))
					tmpWP.Vel.angle = wp["cog"].GetDouble();
				if (wp.HasMember("timestamp"))
					tmpWP.TP.fromString(wp["timestamp"].GetString());
				tmpTraj.trajectory.push_back(tmpWP);
			}			
			re.push_back(tmpTraj);
		}
		//logger.Log({ { Tag::lvl, 5 } }) << "ReadJsonFile_SimpleXYList: Finished reading " << filepath << endl;
		return re;
	}

	template <>
	Json::Document WriteGeoJsonFile<XYList>(
		string outputFilepath, const vector<XYList>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		//GlobalLogger logger;
		//logger.Log({ { Tag::lvl, 5 } }) << "ReadJsonFile_SimpleXYList: Writing " << outputFilepath << endl;

		Json::Document doc;
		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"type": "FeatureCollection",
			"features" : []
		}
		)str");

		for (size_t i = 0; i < routes.size(); i++)
		{
			// make base structure for geojson "Feature"
			Json::Value feature;
			feature.SetObject();
			feature.AddMember("type", Json::Value("Feature"), alloc);
			feature.AddMember("properties", Json::Value(Json::kObjectType), alloc);			
			feature.AddMember("geometry", Json::Value(Json::kObjectType), alloc);
			feature["geometry"].AddMember("type", Json::Value("LineString"), alloc);
			feature["geometry"].AddMember("coordinates", Json::Value(Json::kArrayType), alloc);

			// fill out coordinates
			Json::Value& coordinates = feature["geometry"]["coordinates"];
			for (size_t j = 0; j < routes[i].size(); j++)
			{
				Json::Value apoint(Json::kArrayType);
				apoint.PushBack(Json::Value(routes[i][j].longitude), alloc);
				apoint.PushBack(Json::Value(routes[i][j].latitude), alloc);
				coordinates.PushBack(apoint, alloc);
			}

			// push feature into feature collection
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
	Json::Document WriteGeoJsonFile<XYXtdList>(
		string outputFilepath, const vector<XYXtdList>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
	{
		//GlobalLogger logger;
		//logger.Log({ { Tag::lvl, 5 } }) << "ReadJsonFile_SimpleXYList: Writing " << outputFilepath << endl;

		Json::Document doc;		

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"type": "FeatureCollection",
			"features" : []
		}
		)str");

		for (size_t i = 0; i < routes.size(); i++)
		{
			// make base structure for geojson "Feature"
			Json::Value feature;
			feature.SetObject();
			feature.AddMember("type", Json::Value("Feature"), alloc);
			feature.AddMember("properties", Json::Value(Json::kObjectType), alloc);
			feature["properties"].AddMember("XTDs", Json::Value(Json::kArrayType), alloc);
			feature.AddMember("geometry", Json::Value(Json::kObjectType), alloc);
			feature["geometry"].AddMember("type", Json::Value("LineString"), alloc);
			feature["geometry"].AddMember("coordinates", Json::Value(Json::kArrayType), alloc);

			// fill out coordinates
			Json::Value& coordinates = feature["geometry"]["coordinates"];
			for (size_t j = 0; j < routes[i].size(); j++)
			{
				Json::Value apoint(Json::kArrayType);
				apoint.PushBack(Json::Value(routes[i][j].Pos.longitude), alloc);
				apoint.PushBack(Json::Value(routes[i][j].Pos.latitude), alloc);
				coordinates.PushBack(apoint, alloc);
			}

			// fill out xtds
			Json::Value& XTDs = feature["properties"]["XTDs"];
			for (size_t j = 0; j < routes[i].size(); j++)
			{
				Json::Value apoint(Json::kArrayType);
				apoint.PushBack(Json::Value(routes[i][j].Xtd.xtdPortside), alloc);
				apoint.PushBack(Json::Value(routes[i][j].Xtd.xtdStarboard), alloc);
				XTDs.PushBack(apoint, alloc);
			}

			// push feature into feature collection
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
	Json::Document WriteFelineJsonFile<XYList>(
		string outputFilepath, const vector<XYList>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
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
			routeJson.AddMember("dynamic", Json::Value(Json::kArrayType), alloc);

			// no static data is given, ignore lol

			// add dynamic data
			for (const auto& wp : route)
			{
				Json::Value wpJson(Json::kObjectType);

				// add lat/lon
				wpJson.AddMember("latitude", Json::Value(wp.latitude), alloc);
				wpJson.AddMember("longitude", Json::Value(wp.longitude), alloc);
								
				routeJson["dynamic"].PushBack(wpJson, alloc);
			}

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
	Json::Document WriteFelineJsonFile<XYXtdList>(
		string outputFilepath, const vector<XYXtdList>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
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
			routeJson.AddMember("dynamic", Json::Value(Json::kArrayType), alloc);

			// no static data is given, ignore lol

			// add dynamic data
			for (const auto& wp : route)
			{
				Json::Value wpJson(Json::kObjectType);

				// add lat/lon
				wpJson.AddMember("latitude", Json::Value(wp.Pos.latitude), alloc);
				wpJson.AddMember("longitude", Json::Value(wp.Pos.longitude), alloc);

				// add xtds
				wpJson.AddMember("xtd_p", Json::Value(wp.Xtd.xtdPortside), alloc);
				wpJson.AddMember("xtd_s", Json::Value(wp.Xtd.xtdStarboard), alloc);

				routeJson["dynamic"].PushBack(wpJson, alloc);
			}

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
	Json::Document WriteFelineJsonFile<AisTrajectory<>>(
		string outputFilepath, const vector<AisTrajectory<>>& routes, Json::MemoryPoolAllocator<>& alloc, bool writePretty)
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
			routeJson.AddMember("dynamic", Json::Value(Json::kArrayType), alloc);

			// add static data
			routeJson["static"].AddMember("imo", Json::Value(route.staticInfo.imo), alloc);
			routeJson["static"].AddMember("mmsi", Json::Value(route.staticInfo.mmsi), alloc);
			routeJson["static"].AddMember("loa", Json::Value(route.staticInfo.Dim.L), alloc);
			routeJson["static"].AddMember("beam", Json::Value(route.staticInfo.Dim.B), alloc);
			routeJson["static"].AddMember("draught", Json::Value(route.staticInfo.Dim.T), alloc);

			// add dynamic data
			for (const auto& wp : route.trajectory)
			{
				Json::Value wpJson(Json::kObjectType);

				// add wp data
				wpJson.AddMember("latitude", Json::Value(wp.Pos.latitude), alloc);
				wpJson.AddMember("longitude", Json::Value(wp.Pos.longitude), alloc);
				wpJson.AddMember("sog", Json::Value(wp.Vel.speed), alloc);
				wpJson.AddMember("cog", Json::Value(wp.Vel.angle), alloc);
				string timeStr = wp.TP.toString();				
				wpJson.AddMember("timestamp", Json::Value(timeStr.c_str(), (unsigned int)timeStr.size(), alloc), alloc);

				// 
				routeJson["dynamic"].PushBack(wpJson, alloc);
			}
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
