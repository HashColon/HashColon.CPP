#ifndef HASHCOLON_FELINE_TRAJECTORYCLUSTERING_FELINEJSONIO_HPP
#define HASHCOLON_FELINE_TRAJECTORYCLUSTERING_FELINEJSONIO_HPP

#include <string>
#include <vector>
#include <HashColon/Feline/Types/VoyageSimple.hpp>
namespace HashColon::Feline::IO
{
	std::vector<HashColon::Feline::Types::Simple::XYList> ReadJsonFile_SimpleXYList(std::string filepath);

	void WriteGeoJsonFile(
		std::string outputFilepath, 
		const HashColon::Feline::Types::Simple::XYList& route, 
		bool writePretty = false);

	void WriteGeoJsonFile(
		std::string outputFilepath, 
		const std::vector<HashColon::Feline::Types::Simple::XYList>& routes, 
		bool writePretty = false);
}


#endif