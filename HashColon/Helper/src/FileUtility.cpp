#include <string>
#include <vector>
#include <regex>
#include <boost/filesystem.hpp>
#include <HashColon/Helper/FileUtility.hpp>

using namespace std;

namespace HASHCOLON
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
				using namespace boost::filesystem;
				
				// define a path 
				path aPath(aPathStr);

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
					for (directory_entry& aItem : recursive_directory_iterator(aPath))
					{
						// if aItem is a file
						if (is_regular_file(aItem.path()))
						{
							// if filename matches the regex filter, 
							// add the file's full path to output
							if (regex_match(aItem.path().filename().string(), base_match, filterRegex))
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
				using namespace boost::filesystem;

				// define a path 
				path aDirPath(aDirStr);

				for (directory_entry& aItem : recursive_directory_iterator(aDirPath))
				{
					// if aItem is a file
					if (is_regular_file(aItem.path()))
					{
						// if filename matches the regex filter, 
						// add the file's full path to output
						if (regex_match(aItem.path().filename().string(), base_match, filterRegex))
							re.push_back(aItem.path().string());
					}
				}
			}

			// if additionalFiles exists, add them
			if (!additonalFiles.empty())
				re.insert(re.end(), additonalFiles.begin(), additonalFiles.end());

			return re;
		}

		
	}
}

