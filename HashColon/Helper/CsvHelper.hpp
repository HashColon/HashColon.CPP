#ifndef HASHCOLON_HELPER_CSVHELPER_HPP
#define HASHCOLON_HELPER_CSVHELPER_HPP

#include <unordered_map>
#include <vector>
#include <variant>
#include <string>
#include <functional>

#include <HashColon/Core/Exception.hpp>
#include <HashColon/Helper/StringHelper.hpp>

namespace HashColon::Helper
{
	class CSVValue : public std::variant<bool, long long, long double, std::string>
	{
	public:
		CSVValue() = default;
		CSVValue(const bool& rhs);
		CSVValue(const long long& rhs);
		CSVValue(const long double& rhs);
		CSVValue(const std::string& rhs);
		CSVValue(const char* rhs);
		CSVValue(const CSVValue& rhs) = default;

		CSVValue& operator=(const bool& rhs);
		CSVValue& operator=(const long long& rhs);
		CSVValue& operator=(const long double& rhs);
		CSVValue& operator=(const std::string& rhs);
		CSVValue& operator=(const char* rhs);
		CSVValue& operator=(const CSVValue& rhs) = default;

		HASHCOLON_CLASS_EXCEPTION_DEFINITION(CSVValue);

		template <typename returnT = std::string>
		returnT Get() const;

		template <typename convertT>
		bool ConvertType();
	};

	class CSVRow : public std::unordered_map<std::string, CSVValue>
	{			
	};

	class HashColonCSV : public std::vector<CSVRow>
	{
	public:
		HASHCOLON_CLASS_EXCEPTION_DEFINITION(HashColonCSV);

	protected:
		std::vector<std::string> ColumnNames;
		
	public:
		HashColonCSV() = default;
		HashColonCSV(const HashColonCSV& rhs) : std::vector<CSVRow>(rhs), ColumnNames(rhs.ColumnNames) {};
		HashColonCSV(std::string filepath, std::vector<std::string> columnDefinition = {}, bool ignoreErrorRows = true)
		{
			ReadFromFile(filepath, columnDefinition, ignoreErrorRows);
		}		
		HashColonCSV(std::vector<std::string> colnames) : std::vector<CSVRow>(), ColumnNames(colnames) {};

		bool HasColumnName(std::string name) const;
		std::vector<std::string> GetColumnNames() const;

		void ReadFromFile(std::string filepath, std::vector<std::string> columnDefinition = {}, bool ignoreErrorRows = true);
		void WriteToFile(std::string filepath, bool writeCsvHeader = true) const;

		bool OrderBy(std::vector<std::string> columns, std::vector<bool> isAsc);
		HashColonCSV GetOrderBy(std::vector<std::string> columns, std::vector<bool> isAsc) const;

		HashColonCSV SelectRow(std::function<bool(CSVRow)> filterFunc) const;
		CSVRow FindRow(std::function<bool(CSVRow)> filterFunc) const;
		HashColonCSV SelectCol(std::vector<std::string> columnNames) const;
		HashColonCSV SelectCol(std::vector<std::string> columnNames, std::vector<std::string> newColumnNames) const;

		HashColonCSV GetRemoveDuplicates() const;
		bool RemoveDuplicates();

		template <typename convertT>
		HashColonCSV GetConvertColumnType(const std::string columnName) const;

		template <typename convertT>
		bool ConvertColumnType(const std::string columnName);
	};
}

#endif

#include <HashColon/Helper/impl/CsvHelper_Impl.hpp>