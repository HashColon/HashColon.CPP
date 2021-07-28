#include <unordered_map>
#include <vector>
#include <variant>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <type_traits>

#include <HashColon/Helper/StringHelper.hpp>
#include <HashColon/Helper/CsvHelper.hpp>

using namespace std;

namespace HashColon::Helper
{
	CSVValue::CSVValue(const bool& rhs) : variant<bool, long long, long double, string>(rhs) {};
	CSVValue::CSVValue(const long long& rhs) : variant<bool, long long, long double, string>(rhs) {};
	CSVValue::CSVValue(const long double& rhs) : variant<bool, long long, long double, string>(rhs) {};
	CSVValue::CSVValue(const string& rhs) : variant<bool, long long, long double, string>(rhs) {};
	CSVValue::CSVValue(const char* rhs) : variant<bool, long long, long double, string>(string(rhs)) {};
	CSVValue& CSVValue::operator=(const bool& rhs) { (*this) = rhs; return (*this); };
	CSVValue& CSVValue::operator=(const long long& rhs) { (*this) = rhs; return (*this); };
	CSVValue& CSVValue::operator=(const long double& rhs) { (*this) = rhs; return (*this); };
	CSVValue& CSVValue::operator=(const std::string & rhs) { (*this) = rhs; return (*this); };
	CSVValue& CSVValue::operator=(const char* rhs) { (*this) = string(rhs); return (*this); };


	bool HashColonCSV::HasColumnName(const string name) const
	{
		for (const auto& cname : ColumnNames)
		{
			if (cname == name)
				return true;
		}
		return false;
		/*string _name = name;
		vector<string>::iterator it = find(ColumnNames.begin(), ColumnNames.end(), _name);
		return it != ColumnNames.end();*/
	}

	vector<string> HashColonCSV::GetColumnNames() const { return ColumnNames; }

	void HashColonCSV::ReadFromFile(string filepath, vector<string> columnDefinition, bool ignoreErrorRows)
	{
		// open csv file
		ifstream ifs; ifs.open(filepath);

		// check if the file is opened
		if (!ifs.is_open()) throw Exception("Invalid input csv file.");
		
		// if column names are given, use it
		if (columnDefinition.size() > 0) ColumnNames = columnDefinition;
		// if column names are not given, use first line of the file as csvheader
		else 
		{
			string headerline;
			getline(ifs, headerline);			
			ColumnNames = Split(headerline, ",");
			// trim white spaces
			for (auto& name : ColumnNames) Trim(name);
		}

		// Read each lines
		for (string line; getline(ifs, line); )
		{
			try {
				// split the line by seperator ','
				vector<string> vals = Split(line, ",");

				// check if the number of values from the line coincide with the columns
				if (vals.size() != ColumnNames.size())
					throw Exception("HashColonCSV.ReadFromFile: Number of columns does not match with the number of items in row[" + to_string(size()) + "]");
				
				// new row
				CSVRow newrow;

				// for each value
				for (size_t i = 0; i < vals.size(); i++)
				{
					// Trim white spaces
					Trim(vals[i]);

					// set i-th value of the row
					//variant<bool, long long, string> tmpval(vals[i]);
					//newrow.insert_or_assign(ColumnNames[i], (CSVValue)tmpval);
					CSVValue tmpval = vals[i];
					
					//CSVValue tmpval = "aaa";
					newrow.insert_or_assign(ColumnNames[i], tmpval);					
				}

				// push new value
				push_back(newrow);
			} 
			catch (const Exception& e)
			{
				if (ignoreErrorRows) continue;
				else throw e;
			}
			catch (const CSVValue::Exception& e)
			{
				if (ignoreErrorRows) continue;
				else throw e;
			}
			catch (...)
			{
				throw Exception("HashColonCSV.ReadFromFile: unhandled exception while reading csv file.");
			}

		}
	}

	void HashColonCSV::WriteToFile(std::string filepath, bool writeCsvHeader) const
	{
		// open & check output csv file
		ofstream ofs; ofs.open(filepath);
		if (!ofs.is_open()) throw Exception("Invalid output csv file path.");

		// if writeCsvHeader option is on, write csv header at the first line
		if (writeCsvHeader)
		{
			for (auto column : ColumnNames) ofs << column << ",";
			ofs << endl;
		}

		// write lines
		for (auto row : (*this))
		{
			for (size_t vi = 0; vi < ColumnNames.size(); vi++)
			{
				ofs << row[ColumnNames[vi]].Get<string>() << ",";
			}
			ofs << endl;
		}
	}

	HashColonCSV HashColonCSV::GetOrderBy(vector<string> columns, vector<bool> isAsc) const
	{
		assert(columns.size() > 0);
		assert(columns.size() == isAsc.size());

		HashColonCSV re = (*this);

		auto compareColumns = [columns, isAsc](CSVRow a, CSVRow b)
		{
			for (size_t i = 0; i < columns.size(); i++)
			{
				string key = columns[i];
				if (get_if<bool>(&a[key]) && get_if<bool>(&b[key]))
				{
					if (get<bool>(a[key]) != get<bool>(b[key]))
					{
						return isAsc[i] ? get<bool>(a[key]) < get<bool>(b[key]) : get<bool>(a[key]) > get<bool>(b[key]);
					}
				}
				else if (get_if<long long>(&a[key]) && get_if<long long>(&b[key]))
				{
					if (get<long long>(a[key]) != get<long long>(b[key]))
					{
						return isAsc[i] ? get<long long>(a[key]) < get<long long>(b[key]) : get<long long>(a[key]) > get<long long>(b[key]);
					}
				}
				else if (get_if<long double>(&a[key]) && get_if<long double>(&b[key]))
				{
					if (get<long double>(a[key]) != get<long double>(b[key]))
					{
						return isAsc[i] ? get<long double>(a[key]) < get<long double>(b[key]) : get<long double>(a[key]) > get<long double>(b[key]);
					}
				}
				else if (get_if<string>(&a[key]) && get_if<string>(&b[key]))
				{
					if (get<string>(a[key]) != get<string>(b[key]))
					{
						return isAsc[i] ? get<string>(a[key]) < get<string>(b[key]) : get<string>(a[key]) > get<string>(b[key]);
					}
				}
				else
					throw Exception("Invalid raw type matches.");
			}
			return true;
		};

		try {
			sort(re.begin(), re.end(), compareColumns);
		}
		catch (...)
		{
			re.clear();
		}

		return re;
	}

	bool HashColonCSV::OrderBy(vector<string> columns, vector<bool> isAsc)
	{
		HashColonCSV tmp = GetOrderBy(columns, isAsc);
		if (tmp.size() <= 0) { (*this) = move(tmp); return true; }
		else { return false; }
	}

	HashColonCSV HashColonCSV::SelectRow(function<bool(CSVRow)> filterFunc) const
	{
		HashColonCSV re(ColumnNames);
		for (CSVRow arow : (*this))
		{
			if (filterFunc(arow)) re.push_back(arow);
		}
		return re;
	}

	CSVRow HashColonCSV::FindRow(function<bool(CSVRow)> filterFunc) const
	{
		auto re = find_if(begin(), end(), filterFunc);

		if (re != end()) return (*re);
		else
			throw Exception("Not found");
	}

	HashColonCSV HashColonCSV::SelectCol(vector<string> columnNames, vector<string> newColumnNames) const
	{
		// check if the columnNames & newColumnNames have same number of items
		assert(columnNames.size() == newColumnNames.size());

		// check if all the given columnNames exists in the ColumnNames
		for (auto& colname : ColumnNames)
		{
			assert(find(ColumnNames.begin(), ColumnNames.end(), colname) != ColumnNames.end());
		}
			
		HashColonCSV re(newColumnNames);
		for (auto arow: (*this))
		{
			CSVRow tmprow;
			for (size_t i = 0; i < columnNames.size(); i++)
			{
				tmprow.insert_or_assign(newColumnNames[i], arow[columnNames[i]]);
			}
			re.push_back(arow);
		}
		return re;
	}

	HashColonCSV HashColonCSV::SelectCol(vector<string> columnNames) const
	{
		return SelectCol(columnNames, columnNames);
	}

	HashColonCSV HashColonCSV::GetRemoveDuplicates() const
	{
		HashColonCSV re(ColumnNames);

		for (auto& arow : (*this))
		{
			if (find(re.begin(), re.end(), arow) == re.end())
			{
				re.push_back(arow);
			}
		}
		return re;
	}

	bool HashColonCSV::RemoveDuplicates()
	{
		HashColonCSV tmp;
		try 
		{
			tmp = GetRemoveDuplicates();
		}
		catch (...) { return false; }
		(*this) = move(tmp);
		return true;
	}

}