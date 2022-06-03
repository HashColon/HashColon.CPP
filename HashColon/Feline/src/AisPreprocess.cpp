/*
 * Functions in this file aims for AIS data refinement
 *  !! error rows of csv file is automatically removed from CsvHelper
 *  - Order data by mmsi, imo, timestamp etc
 *  - remove jumping points
 *  - remove non-moving / duplicate points
 */
// HashColon config
#include <HashColon/HashColon_config.h>
// std libraries
#include <algorithm>
#include <cctype>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
// modified external libraries
#include <HashColon/CLI11.hpp>
#include <HashColon/CLI11_JsonSupport.hpp>
// HashColon libraries
#include <HashColon/Helper.hpp>
#include <HashColon/Real.hpp>
#include <HashColon/SingletonCLI.hpp>
#include <HashColon/Table.hpp>
#include <HashColon/ThreadPool.hpp>
#include <HashColon/GeoValues.hpp>
// header file for this source file
#include <HashColon/Feline/AisPreprocess.hpp>

using namespace std;
using namespace HashColon;
using namespace HashColon::Fs;
using namespace HashColon::Feline;

// AisCsvReader
namespace HashColon::Feline
{
	Table AisCsvReader::ReadStaticAisFiles(vector<string> staticFilenames, const vector<string> csvHeader) const
	{
		vector<Table> tmp(staticFilenames.size());
#pragma omp parallel for
		for (size_t i = 0; i < staticFilenames.size(); i++)
		{
			Table tmptable(staticFilenames[i], csvHeader);
			tmptable.RemoveDuplicates({_c.colName.imo, _c.colName.mmsi});
			tmp[i] = tmptable;
		}

		Table re = tmp[0];
		for (size_t i = 1; i < staticFilenames.size(); i++)
		{
			re.insert(re.end(), tmp[i].begin(), tmp[i].end());
			re.RemoveDuplicates({_c.colName.imo, _c.colName.mmsi});
		}
		return re;
	}

	Table AisCsvReader::ReadStaticAisFiles() const
	{
		return ReadStaticAisFiles(GetFilesFromPaths(_c.staticPathAndDirectory, ".*\\.csv"), _c.staticCsvHeader);
	}

	Table AisCsvReader::ReadAisFile(
		const string filename,
		const shared_ptr<Table> &aisStaticTable) const
	{
		// Read AIS CSV files
		Table table;
		if (aisStaticTable)
		{
			Table _table(filename, _c.aisCsvHeader);
			_table.RemoveDuplicates();
			table = InnerJoin(_table, (*aisStaticTable),
							  [&](const Row &a, const Row &b) -> bool
							  {
								  return (a.at(_c.colName.imo) == b.at(_c.colName.imo)) && (a.at(_c.colName.mmsi) == b.at(_c.colName.mmsi));
							  });
		}
		else
		{
			table.ReadFromFile(filename, _c.aisCsvHeader);
			table.RemoveDuplicates();
		}
		return table;
	}

	vector<Table> AisCsvReader::ReadAisFiles(const shared_ptr<Table> &aisStaticTable) const
	{
		vector<string> filenames = GetFilesFromPaths(_c.aisPathAndDirectory, ".*\\.csv");
		vector<Table> re;
		re.reserve(filenames.size());
		for (const auto &name : filenames)
		{
			re.push_back(ReadAisFile(name, aisStaticTable));
		}
		return re;
	}

	map<string, Table> AisCsvReader::GetAisByVessel_withLabel(
		const vector<Table> &tables,
		int threadCnt) const
	{
		map<string, mutex> mutices;
		map<string, Table> reMap;

		// check headers
		for (size_t i = 1; i < tables.size(); i++)
			if (!tables.at(0).HasSameHeaderWith(tables.at(i)))
				throw AisTrajectoryException("GetAisByVessel: headers of input tables do not match.");

		auto processATable = [&](int Tidx, const Table &table) -> void
		{
			for (const auto &row : table)
			{
				// build a key for table in format "mmsi,imo"
				string key = row.at(_c.colName.imo).Get() + "," + row.at(_c.colName.mmsi).Get();

				// if new key is found, create a mutex and a table
				if (mutices.find(key) == mutices.end())
				{
					mutices.try_emplace(key);
					reMap.emplace(key, Table(table.GetColumnNames()));
				}
				{
					lock_guard<mutex> lg(mutices[key]);
					reMap[key].push_back(row);
				}
			}
		};

		ThreadPool tp(threadCnt);
		for (const auto &table : tables)
		{
			tp.Push(processATable, table);
		}
		tp.Wait();

		auto sortByTime = [&](int Tidx, Table &reTable) -> void
		{
			sort(reTable.begin(), reTable.end(),
				 [&](Row a, Row b)
				 {
					 return a.at(_c.colName.timestamp) < b.at(_c.colName.timestamp);
				 });
		};

		ThreadPool tp2(threadCnt);
		for (auto &[key, reTable] : reMap)
		{
			tp2.Push(sortByTime, reTable);
		}
		tp2.Wait();

		return reMap;
	}

	vector<Table> AisCsvReader::GetAisByVessel(
		const vector<Table> &tables,
		int threadCnt) const
	{
		return RemoveLabels(GetAisByVessel_withLabel(tables, threadCnt));
	}

	void AisCsvReader::RemoveInvalidVesselId(Table &table) const
	{
		table.erase(remove_if(
			table.begin(), table.end(),
			[&](const Row &row) -> bool
			{
				// mmsi & imo should only have digits
				string imoStr = row.at(_c.colName.imo).Get();
				string mmsiStr = row.at(_c.colName.mmsi).Get();
				if (!all_of(imoStr.begin(), imoStr.end(), [](char c)
							{ return isdigit(c) != 0; }) ||
					(imoStr.size() > 7) || stoi(imoStr) <= 0)
					return true;
				if (!all_of(mmsiStr.begin(), mmsiStr.end(), [](char c)
							{ return isdigit(c) != 0; }) ||
					(mmsiStr.size() > 9) || stoi(mmsiStr) <= 0)
					return true;
				return false;
			}));
	}

	void AisCsvReader::RemoveJumpAndAnchorPoints(Table &table) const
	{
		Position lastPos{table[0][_c.colName.lon].Get<Real>(), table[0][_c.colName.lat].Get<Real>()};
		TimePoint lastTp;
		lastTp.fromString(table[0][_c.colName.timestamp].Get(), _c.timeFormatStr);
		Table re = table;
		re.clear();
		re.push_back(table[0]);

		for (size_t i = 1; i < table.size(); i++)
		{
			Position currPos{table[i][_c.colName.lon].Get<Real>(), table[i][_c.colName.lat].Get<Real>()};
			TimePoint currTp;
			currTp.fromString(table[i][_c.colName.timestamp].Get(), _c.timeFormatStr);
			Real dist = lastPos.DistanceTo(currPos);
			Real vel = dist / (Real)(chrono::duration_cast<chrono::hours>(currTp - lastTp).count());
			if ((vel < _c.jumpingCriteria) && (vel > _c.anchorCriteria) && (dist > _c.anchorDistCriteria))
			{
				lastPos = currPos;
				re.push_back(table[i]);
			}
		}
		table = re;
	}

	vector<Table> AisCsvReader::RemoveLabels(map<string, Table> labeledCsv) const
	{
		// convert result to vector
		vector<Table> re;
		re.reserve(labeledCsv.size());
		for (auto &[key, val] : labeledCsv)
			re.push_back(val);
		return re;
	}

	vector<Table> AisCsvReader::ReadAndRefineAisFiles() const
	{
		return RemoveLabels(ReadAndRefineAisFiles_withLabel());
	}

	map<string, Table> AisCsvReader::ReadAndRefineAisFiles_withLabel() const
	{
		// Read static files
		shared_ptr<Table> staticAis = nullptr;
		if (!_c.staticPathAndDirectory.empty())
		{
			staticAis = make_shared<Table>(ReadStaticAisFiles());
		}

		// Read ais files
		vector<Table> aisData = ReadAisFiles(staticAis);

		// remove data with invalid vessel id
		for (auto &ais : aisData)
		{
			RemoveInvalidVesselId(ais);
		}

		// Separate ais data by vessels
		map<string, Table> aisByVessel = GetAisByVessel_withLabel(aisData, _c.threadCnt);

		// remove jumping/anchor points
		for (auto &[key, data] : aisByVessel)
		{
			RemoveJumpAndAnchorPoints(data);
		}
		return aisByVessel;
	}

	void AisCsvReader::Initialize(const std::string configFilePath)
	{
		CLI::App *cli = SingletonCLI::GetInstance().GetCLI("Feline.AisPreprocess.AisCsvReader");

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--staticPathAndDirectory", _cDefault.staticPathAndDirectory,
						"List of path to static AIS files or directories with static AIS files. This may be empty array.");
		cli->add_option("--aisPathAndDirectory", _cDefault.aisPathAndDirectory,
						"List of path to AIS files(dynamic) or directories with AIS files.");
		{
			CLI::App *cli_header = SingletonCLI::GetInstance().GetCLI("Feline.AisPreprocess.AisCsvReader.ColumnNames");
			cli_header->add_option("--imo", _cDefault.colName.imo);
			cli_header->add_option("--mmsi", _cDefault.colName.mmsi);
			cli_header->add_option("--lon", _cDefault.colName.lon);
			cli_header->add_option("--lat", _cDefault.colName.lat);
			cli_header->add_option("--timestamp", _cDefault.colName.timestamp);
		}
		cli->add_option("--aisCsvHeader", _cDefault.aisCsvHeader,
						"Header data for AIS data CSV files. If colName data is in the AIS data files, this option can be empty.");
		cli->add_option("--staticCsvHeader", _cDefault.staticCsvHeader,
						"Header data for static AIS data CSV files. If colName data is in the static AIS data files, this option can be empty.");
		cli->add_option("--jumpingCriteria", _cDefault.jumpingCriteria,
						"Velocity criteria for jumping point determination.");
		cli->add_option("--anchorCriteria", _cDefault.anchorCriteria,
						"Velocity criteria for anchoring point determination.");
		cli->add_option("--anchorDistCriteria", _cDefault.anchorDistCriteria,
						"Distance criteria for anchoring point determination.");
		cli->add_option("--timeFormatStr", _cDefault.timeFormatStr,
						"Format string for timestamp data.");
		cli->add_option("--threadCnt", _cDefault.threadCnt, "Number of thread for computation");
	}
}

// AisTrajectoryExtractor
namespace HashColon::Feline
{
	void AisTrajectoryExtraction::Initialize(const std::string configFilePath)
	{
		CLI::App *cli = SingletonCLI::GetInstance().GetCLI("Feline.AisPreprocess.AisTrajectoryExtraction");

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--timeFormatStr", _cDefault.timeFormatStr,
						"Format string for timestamp data & timeIntervalCriteria.");
		cli->add_option("--timeIntervalCriteria", _cDefault.timeIntervalCriteria,
						"Criteria for time-cut in time string format");
	}

	AisTrajectory<> AisTrajectoryExtraction::ConvertCsvToTrajectory(const Table &table) const
	{
		AisTrajectory<> re;
		FillStaticInfo(table, re);
		re.trajectory.reserve(table.size());

		for (size_t i = 0; i < table.size(); i++)
		{
			re.trajectory.push_back(
				GetDynamicWaypoint(table, i));
		}

		return re;
	}

	vector<AisTrajectory<>> AisTrajectoryExtraction::CutTrajectoryByTimeInterval(
		const AisTrajectory<> &traj, Duration criteria) const
	{
		vector<AisTrajectory<>> re;

		auto PushNewTraj = [&](size_t i) -> void
		{
			AisTrajectory<> tmp{traj.staticInfo, {}};
			tmp.trajectory.push_back(traj.trajectory[i]);
		};

		// push first waypoint
		PushNewTraj(0);

		// cut down trajectories
		for (size_t i = 1; i < traj.trajectory.size(); i++)
		{
			const auto &currWP = traj.trajectory[i];
			const auto &lastWP = re.back().trajectory.back();
			Duration timeInterval = currWP.TP - lastWP.TP;

			if (timeInterval > criteria)
			{
				PushNewTraj(i);
			}
			else
			{
				re.back().trajectory.push_back(currWP);
			}
		}

		// if trajectories have only one waypoint, erase.
		re.erase(remove_if(
			re.begin(), re.end(),
			[&](const AisTrajectory<> &traj) -> bool
			{
				return traj.trajectory.size() < 2;
			}));
		return re;
	}

	vector<AisTrajectory<>> AisTrajectoryExtraction::ConvertAll_fromRefinedAisCSV(
		const vector<Table> refinedAisCsv) const
	{
		// convert csv table to trajectory
		vector<AisTrajectory<>> rawAisTraj;
		rawAisTraj.reserve(refinedAisCsv.size());
		for (const auto &csv : refinedAisCsv)
		{
			rawAisTraj.push_back(ConvertCsvToTrajectory(csv));
		}

		// compute time interval criteria
		TimePoint criteriaTP;
		criteriaTP.fromString(_c.timeIntervalCriteria, _c.timeFormatStr);
		Duration criteria = criteriaTP.time_since_epoch();

		// cut down by time interval
		vector<AisTrajectory<>> re;
		for (auto &traj : rawAisTraj)
		{
			vector<AisTrajectory<>> tmpRe =
				CutTrajectoryByTimeInterval(traj, criteria);
			re.insert(re.end(), tmpRe.begin(), tmpRe.end());
		}
		return re;
	}

	void AisTrajectoryExtraction_KMOF::Initialize(const std::string configFilePath)
	{
		CLI::App *cli = SingletonCLI::GetInstance().GetCLI("Feline.AisPreprocess.AisTrajectoryExtraction.KMOF");

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--imo", _cDefault.colName.imo,
						"Header name for imo in KMOF CSV data");
		cli->add_option("--mmsi", _cDefault.colName.mmsi,
						"Header name for mmsi in KMOF CSV data");
		cli->add_option("--lon", _cDefault.colName.lon,
						"Header name for longitude in KMOF CSV data");
		cli->add_option("--lat", _cDefault.colName.lat,
						"Header name for latitude in KMOF CSV data");
		cli->add_option("--timestamp", _cDefault.colName.timestamp,
						"Header name for timestamp in KMOF CSV data");
		cli->add_option("--timestampFormat", _cDefault.timeFormatStr,
						"Format string for timestamp in KMOF CSV data");
	}

	void AisTrajectoryExtraction_KMOF::FillStaticInfo(const Table &table, AisTrajectory<> &traj) const
	{
		traj.staticInfo.imo = table.at(0).at(_c.colName.imo).Get<ShipIDKey>();
		traj.staticInfo.mmsi = table.at(0).at(_c.colName.mmsi).Get<ShipIDKey>();
		traj.staticInfo.Dim.L = table.at(0).at("dimension_a").Get<Real>() + table.at(0).at("dimension_b").Get<Real>();
		traj.staticInfo.Dim.B = table.at(0).at("dimension_c").Get<Real>() + table.at(0).at("dimension_d").Get<Real>();
		traj.staticInfo.Dim.T = table.at(0).at("draught").Get<Real>() / 10.0;
	}

	XYVVaT AisTrajectoryExtraction_KMOF::GetDynamicWaypoint(const Table &table, size_t rowNum) const
	{
		XYVVaT re;
		re.Pos.longitude = table.at(rowNum).at(_c.colName.lon).Get<Real>();
		re.Pos.latitude = table.at(rowNum).at(_c.colName.lat).Get<Real>();
		re.TP.fromString(table.at(rowNum).at(_c.colName.timestamp).Get(), _c.timestampFormatStr);

		if (rowNum == table.size() - 1)
		{
			re.Vel.angle = 0.0;
			re.Vel.speed = 0.0;
		}
		else
		{
			TimePoint nextTP;
			nextTP.fromString(table.at(rowNum + 1).at(_c.colName.timestamp).Get(), _c.timestampFormatStr);
			Position nextPos;
			nextPos.longitude = table.at(rowNum + 1).at(_c.colName.lon).Get<Real>();
			nextPos.latitude = table.at(rowNum + 1).at(_c.colName.lat).Get<Real>();

			re.Vel.angle = GeoDistance::Angle(re.Pos, nextPos);
			re.Vel.speed = GeoDistance::Distance(re.Pos, nextPos) / (Real)(nextTP - re.TP).count();
		}

		return re;
	}

	void AisTrajectoryExtraction_ExactEarth::FillStaticInfo(const Table &table, AisTrajectory<> &traj) const
	{
		traj.staticInfo.imo = table.at(0).at("imo").Get<ShipIDKey>();
		traj.staticInfo.mmsi = table.at(0).at("mmsi").Get<ShipIDKey>();
		traj.staticInfo.Dim.L = table.at(0).at("length").Get<Real>();
		traj.staticInfo.Dim.B = table.at(0).at("width").Get<Real>();
		traj.staticInfo.Dim.T = table.at(0).at("draught").Get<Real>();
	}

	XYVVaT AisTrajectoryExtraction_ExactEarth::GetDynamicWaypoint(const Table &table, size_t rowNum) const
	{
		XYVVaT re;
		re.Pos.longitude = table.at(rowNum).at("longitude").Get<Real>();
		re.Pos.latitude = table.at(rowNum).at("latitude").Get<Real>();
		re.Vel.speed = table.at(rowNum).at("sog").Get<Real>();
		re.Vel.angle = table.at(rowNum).at("cog").Get<Real>();
		re.TP.fromString(table.at(rowNum).at("ts_pos_utc").Get(), "yyyymmddHHMMSS");
		return re;
	}
}
