#ifndef HASHCOLON_HELPER_DBHELPER_HPP
#define HASHCOLON_HELPER_DBHELPER_HPP

#include <HashColon/Helper/CsvHelper.hpp>

namespace HashColon::Helper::DB
{
	using Value = HashColon::Helper::CSVValue;
	using Row = HashColon::Helper::CSVRow;
	using Table = HashColon::Helper::HashColonCSV;

	Table InnerJoin(
		const Table& table1, const Table& table2,
		std::function<bool(const Row&, const Row&)> condition);
}

#endif
