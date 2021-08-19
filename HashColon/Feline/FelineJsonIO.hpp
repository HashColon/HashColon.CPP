#ifndef HASHCOLON_FELINE_FELINEJSONIO_HPP
#define HASHCOLON_FELINE_FELINEJSONIO_HPP

// std libraries
#include <string>
#include <vector>
// dependant external libraries
#include <rapidjson/document.h>

namespace HashColon::Feline::IO
{
	template <typename TList>
	std::vector<TList> ReadFelineJsonFile(const std::string filepath);

	template <typename TList>
	rapidjson::Document WriteFelineJsonFile(
		const std::string outputFilepath,
		const std::vector<TList>& routes,
		rapidjson::MemoryPoolAllocator<>& alloc,
		bool writePretty = false);
		
	template <typename TList>
	void WriteFelineJsonFile(
		const std::string outputFilepath,
		const std::vector<TList>& routes,
		bool writePretty = false)
	{
		rapidjson::MemoryPoolAllocator<> alloc;
		WriteFelineJsonFile(outputFilepath, routes, alloc, writePretty);
	}

	template <typename TList>
	rapidjson::Document WriteGeoJsonFile(
		const std::string outputFilepath,
		const std::vector<TList>& routes,
		rapidjson::MemoryPoolAllocator<>& alloc,
		bool writePretty = false);		

	template <typename TList>
	void WriteGeoJsonFile(
		const std::string outputFilepath,
		const std::vector<TList>& routes,
		bool writePretty = false)
	{
		rapidjson::MemoryPoolAllocator<> alloc;
		WriteGeoJsonFile(outputFilepath, routes, alloc, writePretty);
	}		
}


#endif