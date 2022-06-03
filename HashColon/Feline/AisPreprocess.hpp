#ifndef HASHCOLON_FELINE_AISPREPROCESS
#define HASHCOLON_FELINE_AISPREPROCESS

// std libraries
#include <map>
#include <string>
#include <vector>
// HashColon libraries
#include <HashColon/Exception.hpp>
#include <HashColon/Helper.hpp>
#include <HashColon/Real.hpp>
#include <HashColon/Table.hpp>
#include <HashColon/GeoValues.hpp>

// AisCsvReader
namespace HashColon::Feline
{
	HASHCOLON_NAMED_EXCEPTION_DEFINITION(AisTrajectory);

	class AisCsvReader
	{
	public:
		struct _Params
		{
			std::vector<std::string> staticPathAndDirectory;
			std::vector<std::string> aisPathAndDirectory;
			std::vector<std::string> aisCsvHeader;
			std::vector<std::string> staticCsvHeader;
			struct
			{
				std::string imo;
				std::string mmsi;
				std::string lon;
				std::string lat;
				std::string timestamp;
			} colName;
			HashColon::Real jumpingCriteria; // max velocity criteria btwn two points in km/h
			HashColon::Real anchorCriteria;
			HashColon::Real anchorDistCriteria;
			std::string timeFormatStr;
			int threadCnt;

			_Params() : staticPathAndDirectory(),
						aisPathAndDirectory(),
						aisCsvHeader(),
						staticCsvHeader(),
						colName({"", "", "", "", ""}),
						jumpingCriteria(0),
						anchorCriteria(0),
						anchorDistCriteria(0),
						timeFormatStr(""),
						threadCnt(-1){};
		};

	protected:
		static inline _Params _cDefault;
		const _Params _c;

	public:
		static void Initialize(const std::string configFilePath = "");
		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };

		AisCsvReader(_Params params = _cDefault)
			: _c(_cDefault){};

		HashColon::Table ReadStaticAisFiles(
			std::vector<std::string> staticFilenames, const std::vector<std::string> csvHeader) const;
		HashColon::Table ReadStaticAisFiles() const;
		HashColon::Table ReadAisFile(
			const std::string filename,
			const std::shared_ptr<HashColon::Table> &aisStaticTable) const;
		std::vector<HashColon::Table> ReadAisFiles(
			const std::shared_ptr<HashColon::Table> &aisStaticTable) const;
		std::map<std::string, HashColon::Table> GetAisByVessel_withLabel(
			const std::vector<HashColon::Table> &tables,
			int threadCnt = 10) const;
		std::vector<HashColon::Table> GetAisByVessel(
			const std::vector<HashColon::Table> &tables,
			int threadCnt = 10) const;

		void RemoveInvalidVesselId(HashColon::Table &table) const;
		void RemoveJumpAndAnchorPoints(HashColon::Table &table) const;
		std::vector<HashColon::Table> RemoveLabels(std::map<std::string, HashColon::Table> labeledCsv) const;

		std::vector<HashColon::Table> ReadAndRefineAisFiles() const;
		std::map<std::string, HashColon::Table> ReadAndRefineAisFiles_withLabel() const;
	};
}

// AisTrajectoryExtraction
namespace HashColon::Feline
{
	class AisTrajectoryExtraction
	{
	public:
		struct _Params
		{
			std::string timeFormatStr;
			std::string timeIntervalCriteria;
		};

	protected:
		static inline _Params _cDefault;
		const _Params _c;

	public:
		static void Initialize(const std::string configFilePath = "");
		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };

		AisTrajectoryExtraction(_Params params = _cDefault)
			: _c(_cDefault){};

		AisTrajectory<> ConvertCsvToTrajectory(const HashColon::Table &table) const;
		std::vector<AisTrajectory<>> CutTrajectoryByTimeInterval(
			const AisTrajectory<> &traj, HashColon::Duration criteria) const;
		std::vector<AisTrajectory<>> ConvertAll_fromRefinedAisCSV(
			const std::vector<HashColon::Table> refinedAisCsv) const;

	protected:
		virtual void FillStaticInfo(const HashColon::Table &table, AisTrajectory<> &traj) const = 0;
		virtual XYVVaT GetDynamicWaypoint(const HashColon::Table &table, size_t rowNum) const = 0;
	};

	enum AisDataType
	{
		KMOF,
		ExactEarth
	};

	class AisTrajectoryExtraction_KMOF : public AisTrajectoryExtraction
	{
	public:
		struct _Params : public AisTrajectoryExtraction::_Params
		{
			struct
			{
				std::string imo;
				std::string mmsi;
				std::string lon;
				std::string lat;
				std::string timestamp;
			} colName;
			std::string timestampFormatStr;
		};

	protected:
		static inline _Params _cDefault;
		const _Params _c;

	public:
		static void Initialize(const std::string configFilePath = "");
		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };

		AisTrajectoryExtraction_KMOF()
			: AisTrajectoryExtraction(),
			  _c({AisTrajectoryExtraction::_cDefault, _cDefault.colName}){};

		AisTrajectoryExtraction_KMOF(_Params params)
			: AisTrajectoryExtraction(params), _c(params){};

		void FillStaticInfo(const HashColon::Table &table, AisTrajectory<> &traj) const override final;
		XYVVaT GetDynamicWaypoint(const HashColon::Table &table, size_t rowNum) const override final;
	};

	class AisTrajectoryExtraction_ExactEarth : public AisTrajectoryExtraction
	{
	public:
		AisTrajectoryExtraction_ExactEarth(
			AisTrajectoryExtraction::_Params params = AisTrajectoryExtraction::_cDefault)
			: AisTrajectoryExtraction(params){};

		void FillStaticInfo(const HashColon::Table &table, AisTrajectory<> &traj) const override final;
		XYVVaT GetDynamicWaypoint(const HashColon::Table &table, size_t rowNum) const override final;
	};
}

#endif