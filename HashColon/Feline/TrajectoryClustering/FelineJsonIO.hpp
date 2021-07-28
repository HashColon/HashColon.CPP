#ifndef HASHCOLON_FELINE_TRAJECTORYCLUSTERING_FELINEJSONIO_HPP
#define HASHCOLON_FELINE_TRAJECTORYCLUSTERING_FELINEJSONIO_HPP

#include <string>
#include <vector>
#include <rapidjson/document.h>
#include <HashColon/Feline/Types/VoyageSimple.hpp>
namespace HashColon::Feline::IO
{
	template <typename TList>
	std::vector<TList> ReadJsonFile(std::string filepath);

	/*template 
	std::vector<HashColon::Feline::XYList> 
		ReadJsonFile<HashColon::Feline::XYList>(std::string filepath);*/

	/*template
	std::vector<HashColon::Feline::XYXtdList> 
		ReadJsonFile<HashColon::Feline::XYXtdList>(std::string filepath);*/

	template <typename TList>
	rapidjson::Document WriteGeoJsonFile(
		std::string outputFilepath,
		const std::vector<TList>& route,
		rapidjson::MemoryPoolAllocator<>& alloc,
		bool writePretty = false);		
	
	template <typename TList>
	void WriteGeoJsonFile(
		std::string outputFilepath,
		const std::vector<TList>& route,
		bool writePretty = false)
	{
		rapidjson::MemoryPoolAllocator<> alloc;
		WriteGeoJsonFile(outputFilepath, route, alloc, writePretty);
	}
		
	template <typename TList>
	rapidjson::Document WriteFelineJsonFile(
		std::string outputFilepath,
		const std::vector<TList>& route,
		rapidjson::MemoryPoolAllocator<>& alloc,
		bool writePretty = false);
	
	template <typename TList>
	void WriteFelineJsonFile(
		std::string outputFilepath,
		const std::vector<TList>& route,		
		bool writePretty = false)
	{
		rapidjson::MemoryPoolAllocator<> alloc;
		WriteFelineJsonFile(outputFilepath, route, alloc, writePretty);
	}
	
}


#endif