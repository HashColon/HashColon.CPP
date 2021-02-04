#include <vector>
#include <fstream>
#include <iostream>
#include <cassert>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>

#include <HashColon/Helper/CommonLogger.hpp>
#include <HashColon/Feline/Types/VoyageSimple.hpp>
#include <HashColon/Helper/FileUtility.hpp>

#include <HashColon/Feline/TrajectoryClustering/FelineJsonIO.hpp>

using namespace std;
using namespace HashColon::Feline::Types;
namespace Json = rapidjson;
namespace Helper = HashColon::Helper;

using Tag = HashColon::Helper::LogUtils::Tag;
using GlobalLogger = HashColon::Helper::CommonLogger;

namespace HashColon::Feline::IO
{
	vector<Simple::XYList> ReadJsonFile_SimpleXYList(string filepath)
	{
		// set logger 
		//GlobalLogger logger;
		//logger.Log({ { Tag::lvl, 5 } }) << "ReadJsonFile_SimpleXYList: Reading " << filepath << endl;

		vector<Simple::XYList> re;
		ifstream ifs(filepath);
		Json::IStreamWrapper isw(ifs);

		Json::Document doc;
		doc.ParseStream(isw);

		assert(doc.IsObject());
		assert(doc.HasMember("routes"));
		Json::Value& routes = doc["routes"];
		assert(routes.IsArray());

		const char* lon[3] = { "longitude", "lon", "lng" };
		const char* lat[2] = { "latitude", "lat" };

		for (auto& route : routes.GetArray()) {
			assert(route.HasMember("dynamic"));
			assert(route["dynamic"].IsArray());

			Simple::XYList reRoute;
			for (auto& pt : route["dynamic"].GetArray())
			{
				assert(pt.HasMember(lon[0]) || pt.HasMember(lon[1]) || pt.HasMember(lon[2]));
				assert(pt.HasMember(lat[0]) || pt.HasMember(lat[1]));

				Simple::XY reXY;

				for (int i = 0; i < 3; i++)
					if (pt.HasMember(lon[i]))
					{
						assert((pt[lon[i]].IsDouble()) || (pt[lon[i]].IsString()));
						if (pt[lon[i]].IsDouble())
							reXY.longitude = pt[lon[i]].GetDouble();
						else if (pt[lon[i]].IsString())
							reXY.longitude = stod(pt[lon[i]].GetString());
						break;
					}
				for (int i = 0; i < 2; i++)
					if (pt.HasMember(lat[i]))
					{
						assert((pt[lat[i]].IsDouble()) || (pt[lat[i]].IsString()));
						if (pt[lat[i]].IsDouble())
							reXY.latitude = pt[lat[i]].GetDouble();
						else if (pt[lon[i]].IsString())
							reXY.latitude = stod(pt[lat[i]].GetString());
						break;
					}
				reRoute.push_back(reXY);
			}
			re.push_back(reRoute);
		}

		//logger.Log({ { Tag::lvl, 5 } }) << "ReadJsonFile_SimpleXYList: Finished reading " << filepath << endl;
		return re;
	}

	// Write a Simple::XYList into geojson file. 
	// Only one route!!!!
	void WriteGeoJsonFile(string outputFilepath, const Simple::XYList& route, bool writePretty)
	{
		//GlobalLogger logger;
		//logger.Log({ { Tag::lvl, 5 } }) << "ReadJsonFile_SimpleXYList: Writing " << outputFilepath << endl;
		Json::Document doc;
		Json::Document::AllocatorType& alloc = doc.GetAllocator();

		// make base structure for geojson "feature"
		doc.Parse(R"str(
			{
				"type": "Feature",
				"properties" : {},
				"geometry" : {
					"type" : "LineString",
					"coordinates" : []
			} 
		)str");

		Json::Value& coordinates = doc["geometry"]["coordinates"];

		// fill out coordinates 
		for (int i = 0; i < route.size(); i++)
		{
			Json::Value apoint(Json::kArrayType);
			apoint.PushBack(Json::Value(route[i].longitude), alloc);
			apoint.PushBack(Json::Value(route[i].latitude), alloc);
			coordinates.PushBack(apoint, alloc);
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
		//logger.Log({ { Tag::lvl, 5 } }) << "ReadJsonFile_SimpleXYList: Finished writing " << outputFilepath << endl;
	}

	void WriteGeoJsonFile(string outputFilepath, const vector<Simple::XYList>& routes, bool writePretty)
	{
		//GlobalLogger logger;
		//logger.Log({ { Tag::lvl, 5 } }) << "ReadJsonFile_SimpleXYList: Writing " << outputFilepath << endl;

		Json::Document doc;
		Json::Document::AllocatorType& alloc = doc.GetAllocator();

		// make base structure for geojson "FeatureCollection"
		doc.Parse(R"str(
		{
			"type": "FeatureCollection",
			"features" : []
		}
		)str");

		for (int i = 0; i < routes.size(); i++)
		{
			// make base structure for geojson "Feature"
			Json::Value feature;
			feature.SetObject();
			feature.AddMember("type", Json::Value("Feature"), alloc);
			feature.AddMember("properties", Json::Value(Json::kObjectType), alloc);
			feature.AddMember("geometry", Json::Value(Json::kObjectType), alloc);
			feature["geometry"].AddMember("type", Json::Value("LineString"), alloc);
			feature["geometry"].AddMember("coordinates", Json::Value(Json::kArrayType), alloc);

			Json::Value& coordinates = feature["geometry"]["coordinates"];

			// fill out coordinates
			for (int j = 0; j < routes[i].size(); j++)
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
		//logger.Log({ { Tag::lvl, 5 } }) << "ReadJsonFile_SimpleXYList: Finished writing " << outputFilepath << endl;
	}

}


