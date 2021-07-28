#include <cassert>
#include <string>
#include <mutex>
#include <algorithm>
#include <HashColon/Helper/DBHelper.hpp>

using namespace std;

namespace HashColon::Helper::DB
{
	Table InnerJoin(
		const Table& table1, const Table& table2,
		std::function<bool(const Row&, const Row&)> condition)
	{
		// assert size of the tables
		assert(table1.size() > 0);
		assert(table2.size() > 0);
		if (table1.size() <= 0 || table2.size() <= 0)
			throw Table::Exception("InnerJoin: input table should have at least 1 row.");

		// set column names for return rows
		Row sampleRow;
		Row t1Sample = table1[0];
		Row t2Sample = table2[0];
		sampleRow.merge(t1Sample); sampleRow.merge(t2Sample);
		vector<string> reColnames(sampleRow.size());
		for (auto kv : sampleRow)
			reColnames.push_back(kv.first);
		Table re(reColnames);

		#pragma omp parallel for
		for (size_t i = 0; i < table1.size(); i++)
		{
			Row row1 = table1[i];
			for (size_t j = 0; j < table2.size(); j++)
			{
				Row row2 = table2[j];
				if (condition(row1, row2)) 
				{
					Row reRow;					
					reRow.merge(row1); reRow.merge(row2);
					re.push_back(reRow);
				}
			}
		}
		return re;
	}
}