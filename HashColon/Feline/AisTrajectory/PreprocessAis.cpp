/*
* Functions in this file aims for AIS data refinement
*  !! error rows of csv file is automatically removed from CsvHelper
*  - Order data by mmsi, imo, timestamp etc
*  - remove jumping points
*  - remove non-moving / duplicate points
* 
*/
#include <memory>
#include <string>
#include <vector>
#include <HashColon/Helper/DBHelper.hpp>
#include <HashColon/Feline/Types/ValueTypes.hpp>
#include <HashColon/Feline/Types/VoyageSimple.hpp>

using namespace std;
using namespace HashColon;
using namespace HashColon::Helper;
using namespace HashColon::Feline;

DB::Table ReadStaticAISFiles(vector<string> staticFilenames, vector<string> csvHeader)
{
	vector<DB::Table> tmp(staticFilenames.size());
	#pragma omp parallel for
	for (size_t i = 0; i < staticFilenames.size(); i++)
	{	
		DB::Table tmptable(staticFilenames[i], csvHeader);
		tmptable.RemoveDuplicates();
		tmp[i] = tmptable;
	}

	DB::Table re = tmp[0];
	for (size_t i = 1; i < staticFilenames.size(); i++)
	{		
		re.insert(re.end(), tmp[i].begin(), tmp[i].end());
		re.RemoveDuplicates();
	}
	return re;
}

DB::Table ReadAISFile(
	const string filename, 
	const shared_ptr<DB::Table>& aisStaticTable,
	const vector<string> csvHeader,
	const string header_IMO, const string header_MMSI)
{
	// Read AIS CSV files
	DB::Table table;
	if (aisStaticTable)
	{
		DB::Table _table(filename, csvHeader);
		_table.RemoveDuplicates();		
		table = DB::InnerJoin(_table, (*aisStaticTable),
			[&header_IMO, &header_MMSI](const DB::Row& a, const DB::Row& b)->bool
			{
				return (a.at(header_IMO) == b.at(header_IMO))
					&& (a.at(header_MMSI) == b.at(header_MMSI));
			});
	}
	else
	{
		table.ReadFromFile(filename, csvHeader);		
	}		
}

struct AisTrajectoryHeaders
{
	string lon;
	string lat;
	string imo;
	string mmsi;
	string sog;
	string heading;
	string timestamp;
};
struct AisTrajectory
{
	string imo;
	string mmsi;
	XYVVaTList trajectory;
};

const AisTrajectoryHeaders stdHeaders = { "lon", "lat", "imo", "mmsi", "sog", "heading", "timestamp" };

vector<AisTrajectory> GetAisTrajectories(
	const DB::Table& aisData, 	
	const AisTrajectoryHeaders _h,
	const string timeFormatStr)
{
	DB::Table table = aisData.SelectCol(
		{ _h.lon, _h.lat, _h.imo, _h.mmsi, _h.sog, _h.heading, _h.timestamp },
		{ stdHeaders.lon, stdHeaders.lat, stdHeaders.imo, stdHeaders.mmsi, stdHeaders.sog, stdHeaders.heading, stdHeaders.timestamp }
	);	
	table.OrderBy({ "imo", "mmsi", "timestamp" }, { true,true,true });

	// return value  
	vector<AisTrajectory> re;

	// lambda functions for assignment 
	auto NewPoint = [&aisData, &timeFormatStr](size_t i)->XYVVaT {
		XYVVaT tmpPoint;
		tmpPoint._Pos.longitude = aisData.at(i).at(stdHeaders.lon).Get<Real>();
		tmpPoint._Pos.latitude = aisData.at(i).at(stdHeaders.lat).Get<Real>();
		tmpPoint._Vel.speed = aisData.at(i).at(stdHeaders.sog).Get<Real>();
		tmpPoint._Vel.angle = aisData.at(i).at(stdHeaders.heading).Get<Real>();
		tmpPoint._TP.fromString(timeFormatStr, aisData.at(i).at(stdHeaders.timestamp).Get());
		return tmpPoint;
	};
	auto NewTrajectory = [&aisData](size_t i)->AisTrajectory {
		AisTrajectory tmp;
		tmp.imo = aisData.at(i).at(stdHeaders.imo).Get();
		tmp.mmsi = aisData.at(i).at(stdHeaders.mmsi).Get();
		return tmp;
	};

	// read rows in sequence
	for (size_t i = 0; i < aisData.size(); i++)
	{
		//// if row[i] has invalid value
		//if (IsInvalidValue(i))
		//{
		//	continue;
		//}
		//else if(IsVesselChanged(i))
		//{
		//	AisTrajectory tmpTraj = NewTrajectory(i);
		//	tmpTraj.trajectory.push_back(NewPoint(i));
		//	re.push_back(tmpTraj);
		//}
		//else if (IsJumpingPoint(i))
		//{
		//	continue;
		//}
		//else if (IsNoMovePoint(i))
		//{
		//	continue;
		//}
		//else if (IsLongTimeInterval(i))
		//{
		//	AisTrajectory tmpTraj = NewTrajectory(i);			
		//	tmpTraj.trajectory.push_back(NewPoint(i));
		//	re.push_back(tmpTraj);
		//}
		//else
		//{
		//	re.back().trajectory.push_back(NewPoint(i));
		//}

		
	}
}
