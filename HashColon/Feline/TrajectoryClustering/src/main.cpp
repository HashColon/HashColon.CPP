#include <HashColon/Feline/Feline_config.h>
#include <cmath>
#include <vector>
#include <functional>
#include <any>
#include <string>
#include <limits>

#include <iostream>

#include <Eigen/Eigen>

#include <HashColon/Helper/Real.hpp>
#include <HashColon/Helper/SingletonCLI.hpp>
#include <HashColon/Helper/FileUtility.hpp>
#include <HashColon/Helper/CommonLogger.hpp>

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
} _c;

void run()
{	
	GlobalLogger logger;
	logger.Log({ {Tag::lvl, 1} }) << "Start!" << endl;	
	string dir = "/home/cadit/WTK/WTK_data/ExactEarth/result/in";
	string outprefix = "/home/cadit/WTK/WTK_data/ExactEarth/result/euclidean/cluster_";
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

	DistanceMeasureBase<Simple::XYList>::Ptr euclidean = make_shared<Euclidean>(Euclidean::_Params{true });
	DistanceMeasureBase<Simple::XYList>::Ptr hausdorff = make_shared<Hausdorff>(Hausdorff::_Params{true });
	DistanceMeasureBase<Simple::XYList>::Ptr lcss = make_shared<LCSS>(LCSS::_Params{ {true}, 30, 3 });
	DistanceMeasureBase<Simple::XYList>::Ptr merge = make_shared<Merge>(Merge::_Params{true});
	

	NJW<Simple::XYList>::_Params njwparams;
	njwparams.similaritySigma = 50;
	njwparams.k = 2;
	njwparams.kmeansEpsilon = 0.01;
	njwparams.kmeansIteration = 100;
	
	//NJW<Simple::XYList> njw(euclidean, njwparams);
	//NJW<Simple::XYList> njw(hausdorff, njwparams);
	//NJW<Simple::XYList> njw(lcss, njwparams);
	NJW<Simple::XYList> njw(merge, njwparams);

	shared_ptr<vector<size_t>> labels = make_shared<vector<size_t>>();
	njw.TrainModel(routes, labels);
		
	vector<vector<Simple::XYList>> re;
	re.resize(njwparams.k);
	for (size_t i = 0; i < routes.size(); i++)
	{
		re[labels->at(i)].push_back(routes[i]);
	}

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

	GlobalLogger::Initialize();
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
				"./config/CommonLogger.json",
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