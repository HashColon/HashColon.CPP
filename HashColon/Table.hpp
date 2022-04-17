#ifndef HASHCOLON_TABLE
#define HASHCOLON_TABLE

// std libraries
#include <functional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
// HashColon libraries
#include <HashColon/Exception.hpp>
#include <HashColon/Helper.hpp>

// CSV types. value, row, table
namespace HashColon
{
	class CSVValue : public std::variant<bool, long long, long double, std::string>
	{
	public:
		CSVValue() = default;
		CSVValue(const bool &rhs);
		CSVValue(const long long &rhs);
		CSVValue(const long double &rhs);
		CSVValue(const std::string &rhs);
		CSVValue(const char *rhs);
		CSVValue(const CSVValue &rhs) = default;

		CSVValue &operator=(const bool &rhs);
		CSVValue &operator=(const long long &rhs);
		CSVValue &operator=(const long double &rhs);
		CSVValue &operator=(const std::string &rhs);
		CSVValue &operator=(const char *rhs);
		CSVValue &operator=(const CSVValue &rhs) = default;

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
		HashColonCSV(const HashColonCSV &rhs) : std::vector<CSVRow>(rhs), ColumnNames(rhs.ColumnNames){};
		HashColonCSV(std::string filepath, std::vector<std::string> columnDefinition = {}, bool ignoreErrorRows = true)
		{
			ReadFromFile(filepath, columnDefinition, ignoreErrorRows);
		}
		HashColonCSV(std::vector<std::string> colnames) : std::vector<CSVRow>(), ColumnNames(colnames){};

		bool HasColumnName(std::string name) const;
		std::vector<std::string> GetColumnNames() const;
		bool HasSameHeaderWith(const HashColonCSV &table) const;

		void ReadFromFile(std::string filepath, std::vector<std::string> columnDefinition = {}, bool ignoreErrorRows = true);
		void WriteToFile(std::string filepath, bool writeCsvHeader = true) const;

		bool OrderBy(std::vector<std::string> columns, std::vector<bool> isAsc);
		HashColonCSV GetOrderBy(std::vector<std::string> columns, std::vector<bool> isAsc) const;

		HashColonCSV SelectRow(std::function<bool(CSVRow)> filterFunc) const;
		CSVRow FindRow(std::function<bool(CSVRow)> filterFunc) const;
		HashColonCSV SelectCol(std::vector<std::string> columnNames) const;
		HashColonCSV SelectCol(std::vector<std::string> columnNames, std::vector<std::string> newColumnNames) const;

		HashColonCSV GetRemoveDuplicates() const;
		HashColonCSV GetRemoveDuplicates(const std::vector<std::string> checkCols) const;
		bool RemoveDuplicates();
		bool RemoveDuplicates(const std::vector<std::string> checkCols);

		template <typename convertT>
		HashColonCSV GetConvertColumnType(const std::string columnName) const;

		template <typename convertT>
		bool ConvertColumnType(const std::string columnName);

		void ChangeColumnName(const std::string targetColName, const std::string newColName);
		void ChangeColumnName(const std::vector<std::string> &targetColNames, const std::vector<std::string> &newColNames);
		HashColonCSV GetChangeColumnName(const std::string targetColName, const std::string newColName);
		HashColonCSV GetChangeColumnName(const std::vector<std::string> &targetColNames, const std::vector<std::string> &newColNames);

		friend HashColonCSV operator+(const HashColonCSV &a, const HashColonCSV &b);
	};
}

// Table
namespace HashColon
{
	using Value = HashColon::CSVValue;
	using Row = HashColon::CSVRow;
	using Table = HashColon::HashColonCSV;

	Table InnerJoin(
		const Table &table1, const Table &table2,
		std::function<bool(const Row &, const Row &)> condition);
}

#endif

#include <HashColon/impl/Table_Impl.hpp>