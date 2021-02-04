#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <filesystem>
#include <stack>
//#include <boost/filesystem.hpp>
#include <HashColon/Helper/FileUtility.hpp>

using namespace std;

namespace HashColon
{
	namespace Helper
	{
		// purpose
		// describe varied file name as 

		vector<string> GetFilesInDirectory(string iDir, vector<string> additonalFiles, string filterRegexStr)
		{
			vector<string> tmp; tmp.clear();
			tmp.push_back(iDir);
			return GetFilesInDirectories(tmp, additonalFiles, filterRegexStr);
		}

		vector<string> GetFilesFromPaths(vector<string>& iPaths, string filterRegexStr)
		{
			vector<string> re; re.clear();

			std::regex filterRegex(filterRegexStr);
			std::smatch base_match;

			// iterate 
			for (string& aPathStr : iPaths)
			{
				// we are not checking the validity of directory cause we have done it with cli11

				// we are using boost::filesystem library here
				//using namespace boost::filesystem;				
				using namespace std::filesystem;
				
				// define a path 
				path aPath = aPathStr.c_str();

				// if current path is a file
				if (is_regular_file(aPath))
				{
					// add current file's path
					re.push_back(aPath.string());
				}
				// else if current path is a directory
				else if (is_directory(aPath))
				{
					// iterate 
					for (const directory_entry aItem : recursive_directory_iterator(aPath.c_str()))
					{
						// if aItem is a file
						if (is_regular_file(aItem.path()))
						{
							// if filename matches the regex filter, 
							// add the file's full path to output
							string filenamestr = aItem.path().filename().string();
							if (regex_match(filenamestr, base_match, filterRegex))
								re.push_back(aItem.path().string());
						}
					}
				}
				// else, ignore current path
				else
				{
					// TODO: add message("file/directory not found") here
					continue;
				}
				
			}
			return re;
		}

		vector<string> GetFilesInDirectories(vector<string>& iDirs, vector<string> additonalFiles, string filterRegexStr)
		{
			vector<string> re; re.clear();

			std::regex filterRegex(filterRegexStr);
			std::smatch base_match;

			// iterate 
			for (string& aDirStr : iDirs)
			{
				// we are not checking the validity of directory cause we have done it with cli11

				// we are using boost::filesystem library here
				//using namespace boost::filesystem;
				using namespace std::filesystem;

				// define a path 
				path aDirPath = aDirStr.c_str();

				for (const directory_entry aItem : recursive_directory_iterator(aDirPath))
				{
					// if aItem is a file
					if (is_regular_file(aItem.path()))
					{
						// if filename matches the regex filter, 
						// add the file's full path to output
						string filenamestr = aItem.path().filename().string();
						if (regex_match(filenamestr, base_match, filterRegex))
							re.push_back(aItem.path().string());
					}
				}
			}

			// if additionalFiles exists, add them
			if (!additonalFiles.empty())
				re.insert(re.end(), additonalFiles.begin(), additonalFiles.end());

			return re;
		}

		inline string RefinedAbsolutePathStr(string iPathstring)
		{
			using namespace std::filesystem;
			return canonical(absolute(path(iPathstring))).string();
		}

		bool BuildDirectoryStructure(string iDirectoryPathString)
		{
			using namespace std::filesystem;
			path p = absolute(path(iDirectoryPathString));
			if (!exists(p))
				return create_directories(p);
			else return true;
		}

		
	}
}

