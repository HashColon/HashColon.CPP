#ifndef HASHCOLON_HELPER_CSVUTILS_IMPL
#define HASHCOLON_HELPER_CSVUTILS_IMPL

#include <HashColon/Helper/CsvHelper.hpp>

namespace HashColon::Helper
{	
	template <typename returnT = std::string>
	returnT CSVValue::Get() const
	{		
		using namespace std;
		if (get_if<bool>(this))
		{
			auto v = get<bool>(*this);
			if constexpr (is_convertible_v<string, returnT>) return v ? "true" : "false";
			else
				return (returnT)v;
		}
		else if (get_if<long long>(this))
		{
			auto v = get<long long>(*this);
			if constexpr (is_convertible_v<string, returnT>) return to_string(v);
			else
				return (returnT)v;
		}
		else if (get_if<long double>(this))
		{
			auto v = get<long double>(*this);
			if constexpr (is_convertible_v<string, returnT>) return to_string(v);
			else
				return (returnT)v;
		}
		else if (get_if<string>(this))
		{
			auto v = get<string>(*this);
			if constexpr (is_same_v<bool, returnT>)
			{
				string tmpv = ToLowerCopy(TrimCopy(v));
				if (tmpv == "true") return true;
				else if (tmpv == "false") return false;
				else
					throw Exception("Invalid string-boolean value" + tmpv);
			}
			else if constexpr (is_integral_v<returnT>) return atoi(v.c_str());
			else if constexpr (is_floating_point_v<returnT>) return atof(v.c_str());
			else
				return (returnT)v;
		}
		else
			throw Exception("Invalid csv value");
	}

	template <typename convertT>
	bool CSVValue::ConvertType() 
	{
		using namespace std;
		if constexpr (is_same_v<convertT, bool>)
		{
			bool converted;
			try {
				converted = Get<bool>();
			}
			catch (Exception& e) { return false; }
			catch (std::invalid_argument& e) { return false; }
			(*this) = converted;
			return true;
		}
		else if constexpr (is_integral_v<convertT>)
		{
			long long converted;
			try {
				converted = Get<long long>();
			}
			catch (Exception& e) { return false; }
			catch (std::invalid_argument& e) { return false; }
			(*this) = converted;
			return true;
		}
		else if constexpr (is_floating_point_v<convertT>)
		{
			long double converted;
			try {
				converted = Get<long double>();
			}
			catch (Exception& e) { return false; }
			catch (invalid_argument& e) { return false; }
			(*this) = converted;
			return true;
		}
		else if constexpr (is_convertible_v<convertT, string>)
		{
			string converted;
			try {
				converted = Get<string>();
			}
			catch (Exception& e) { return false; }
			catch (invalid_argument& e) { return false; }
			(*this) = converted;
			return true;
		}
		else
		{
			return false;
		}
	}

	template <typename convertT>
	HashColonCSV HashColonCSV::GetConvertColumnType(const std::string columnName) const
	{		
		using namespace std;

		// copy return csv
		HashColonCSV re = (*this);
		bool isSuccess = true;

		for (CSVRow& row : re)
		{
			if constexpr (is_same_v<convertT, bool>)
			{
				bool converted;
				try {
					converted = row[columnName].Get<bool>();
				}
				catch (Exception& e) { isSuccess = false; break; }
				catch (std::invalid_argument& e) { isSuccess = false; break; }
				row[columnName] = converted;
			}
			else if constexpr (is_integral_v<convertT>)
			{
				long long converted;
				try {
					converted = row[columnName].Get<long long>();
				}
				catch (Exception& e) { isSuccess = false; break; }
				catch (std::invalid_argument& e) { isSuccess = false; break; }
				row[columnName] = converted;
			}
			else if constexpr (is_floating_point_v<convertT>)
			{
				long double converted;
				try {
					converted = row[columnName].Get<long double>();
				}
				catch (Exception& e) { isSuccess = false; break; }
				catch (std::invalid_argument& e) { isSuccess = false; break; }
				row[columnName] = converted;
			}
			else if constexpr (is_convertible_v<convertT, string>)
			{
				string converted;
				try {
					converted = row[columnName].Get<string>();
				}
				catch (Exception& e) { isSuccess = false; break; }
				catch (std::invalid_argument& e) { isSuccess = false; break; }
				row[columnName] = converted;
			}
			else
			{
				isSuccess = false; break;
			}
		}

		if (!isSuccess) re.clear();
		return re;
	}

	template <typename convertT>
	bool HashColonCSV::ConvertColumnType(const std::string columnName)
	{
		HashColonCSV converted = GetConvertColumnType<convertT>(columnName);
		if (converted.size() == 0) return false;
		else
		{
			(*this) = converted; return true;
		}
	}
}

#endif
