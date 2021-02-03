#include <HashColon/Feline/Feline_config.h>
#include <cmath>
#include <vector>
#include <functional>
#include <any>
#include <string>
#include <limits>
#include <unordered_map>

#include <iostream>

#include <Eigen/Eigen>

#include <HashColon/Helper/Real.hpp>
#include <HashColon/Helper/SingletonCLI.hpp>
#include <HashColon/Helper/FileUtility.hpp>
#include <HashColon/Helper/CommonLogger.hpp>
#include <HashColon/Helper/Exception.hpp>

#include <HashColon/Clustering/ClusteringBase.hpp>
#include <HashColon/Feline/TrajectoryClustering/TrajectoryDistanceMeasure.hpp>

#include <HashColon/Clustering/NJW.hpp>
#include <HashColon/Feline/Types/VoyageSimple.hpp>
#include <HashColon/Feline/TrajectoryClustering/FelineJsonIO.hpp>

using namespace std;
using namespace Eigen;
using namespace HashColon::Clustering;
using namespace HashColon::Helper;
using namespace HashColon::Feline;
using namespace HashColon::Feline::Types;
using namespace HashColon::Feline::TrajectoryClustering::DistanceMeasure;

using Tag = HashColon::Helper::LogUtils::Tag;
using GlobalLogger = HashColon::Helper::CommonLogger;
using SingletonCLI = HashColon::Helper::SingletonCLI;

static struct _Params
{
	string inputDir;
	string outputDir;
	string outputPrefix;
	string clusteringMethodName;
	string measuringMethodName;
	bool enableUniformSampling;
	size_t UniformSamplingNumber;
} _c;

HASHCOLON_NAMED_EXCEPTION_DEFINITION(main);

void run()
{	
	GlobalLogger logger;
	logger.Log({ {Tag::lvl, 1} }) << "Start!" << endl;	
	string dir = _c.inputDir;
	string outprefix = _c.outputDir + "/" + _c.clusteringMethodName + "/" + _c.measuringMethodName + "/" + _c.outputPrefix;
	vector<string> filepaths = GetFilesInDirectory(dir);	

	vector<vector<Simple::XYList>> routes_raw;
	routes_raw.resize(filepaths.size());
	vector<Simple::XYList> routes;

	logger.Log({ {Tag::lvl, 1} }) << "Parsing start." << endl;
	
	// parse json to routes	
	#pragma omp parallel for 
	for (int i = 0; i < filepaths.size(); i ++)
	{
		routes_raw[i] = IO::ReadJsonFile_SimpleXYList(filepaths[i]);
	}

	// merge routes into a array
	int size = 0;
	for (auto& raw : routes_raw) { size += raw.size(); }
	routes.reserve(size);
	for (auto& raw : routes_raw) { routes.insert(routes.end(), raw.begin(), raw.end()); }
	routes_raw.clear();

	logger.Log({ {Tag::lvl, 1} }) << "Parsing finished. " << routes.size() << " routes." << endl;

	// pick similarity/distance method
	unordered_map<string, DistanceMeasureBase<Simple::XYList>::Ptr> measuringMethodList
	{
		{"Euclidean", make_shared<Euclidean>()},
		{"Hausdorff", make_shared<Hausdorff>()},
		{"LCSS", make_shared<LCSS>()},
		{"Merge", make_shared<Merge>()}
	};
	DistanceMeasureBase<Simple::XYList>::Ptr measuringMethod;
	try {
		measuringMethod = measuringMethodList[_c.measuringMethodName];
	}
	catch (out_of_range e)
	{
		throw mainException(
			"Invalid distance/similarity measuring method name. Check option --measuringMethodName.",
			__CODEINFO__);
	}

	// pick clustering method
	unordered_map<string, DistanceBasedClustering<Simple::XYList>::Ptr> clusteringMethodList
	{
		{"NJW", make_shared<NJW<Simple::XYList>>(measuringMethod)}
	};
	DistanceBasedClustering<Simple::XYList>::Ptr clusteringmethod;
	try {
		clusteringmethod = clusteringMethodList[_c.clusteringMethodName];
	}
	catch (out_of_range e)
	{
		throw mainException(
			"Invalid clustering method name. Check option --clusteringMethodName.",
			__CODEINFO__);
	}

	if (_c.enableUniformSampling)
	{
		#pragma openmp parallel for
		for (size_t i = 0; i < routes.size(); i++)
		{
			routes[i] = routes[i].GetNormalizedXYList(_c.UniformSamplingNumber);
		}
	}

	shared_ptr<vector<size_t>> labels = make_shared<vector<size_t>>();
	clusteringmethod->TrainModel(routes, labels);
		
	vector<vector<Simple::XYList>> re;
	re.resize(clusteringmethod->GetNumOfClusters());
	for (size_t i = 0; i < routes.size(); i++)
	{
		re[labels->at(i)].push_back(routes[i]);
	}

	#pragma openmp parallel for
	for (size_t i = 0; i < re.size(); i++)
	{
		string refile = outprefix + to_string(i) + ".json";
		IO::WriteGeoJsonFile(refile, re[i]);
	}

	logger.Log({ {Tag::lvl, 1} }) << "Finished." << endl;

}

static void Initialize()
{	
	// register config files
	SingletonCLI::Initialize();

	GlobalLogger::Initialize("./config/CommonLogger.json");
	NJW<Simple::XYList>::Initialize();
	TrajectoryDistanceMeasureBase::Initialize();	
	LCSS::Initialize();	

	CLI::App* cli = SingletonCLI::GetInstance().GetCLI();	

	cli->add_option("--inputDir", _c.inputDir);
	cli->add_option("--outputDir", _c.outputDir);
	cli->add_option("--outputPrefix", _c.outputPrefix);
	cli->add_option("--clusteringMethodName", _c.clusteringMethodName);
	cli->add_option("--measuringMethodName", _c.measuringMethodName);	
	cli->add_option("--enableUniformSampling", _c.enableUniformSampling);
	cli->add_option("--UniformSamplingNumber", _c.UniformSamplingNumber);

	// Parse additional options from command line.
	SingletonCLI::GetInstance().GetCLI()->set_help_all_flag("--help-all");

	cli->callback(run);
}

int main(int argc, char** argv)
{	
	Initialize();

	try
	{	
		SingletonCLI::GetInstance().Parse(
			argc, argv,
			{	
				//"./config/CommonLogger.json",
				"./config/TrajectoryClustering.json"
			});
	}
	catch (const CLI::Error& e)
	{
		SingletonCLI::GetInstance().GetCLI()->exit(e);
	}
	catch (const HashColon::Exceptions::Exception& e)
	{
		CommonLogger logger;
		logger.Error({ { Tag::file, e.file()}, { Tag::func, e.func() }, { Tag::line, e.line() } }) << e.what() << std::endl;
		/*string _file = e.file();
		string _func = e.func();
		int _line = e.line();
		string _what = e.what();
		logger.Error({ { Tag::file, _file }, { Tag::func, _func }, { Tag::line, _line } }) << _what << std::endl;*/
	}
	
}