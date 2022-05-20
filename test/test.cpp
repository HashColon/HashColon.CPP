#include <HashColon/SingletonCLI.hpp>
#include <HashColon/Log.hpp>
#include <iostream>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace HashColon;

void coutTest(ofstream &ofs1)
{
    auto now = system_clock::now();
    auto now_time_t = system_clock::to_time_t(now);
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    // print timestamp and tag
    ofs1 << "[" << put_time(localtime(&now_time_t), "%F %T.") << setfill('0') << setw(3) << ms.count() << "]"
         << "[ Log ]: "
         << "AAAA" << endl;
}

void globalTest()
{
    GlobalLogger.Log << "AAAA" << endl;
}

void unittest_CommonLogger()
{
    size_t N = 1;

    CommonLogger::_Params params_local, params_global;
    ofstream ofs1("buf1.txt");
    // shared_ptr<ofstream> ofs2 = make_shared<ofstream>("buf2.txt");
    shared_ptr<ofstream> ofs3 = make_shared<ofstream>("buf3.txt");
    // params_local.logStreams.push_back(ofs2);
    params_global.logStreams.push_back(ofs3);
    GlobalLogger.Log.Stream().StreamList().clear();
    GlobalLogger.Log.Stream().StreamList().push_back(ofs3);

    CommonLogger stamp;

    stamp.Log() << "cout start" << endl;
    for (size_t i = 0; i < N; i++)
        coutTest(ofs1);
    stamp.Log() << "cout end" << endl;

    // stamp.Log() << "common start" << endl;
    // for (size_t i = 0; i < N; i++)
    // {
    //     CommonLogger log2(params_local);
    //     for (size_t j = 0; j < 10; j++)
    //     {
    //         log2.Log() << "AAAA" << endl;
    //     }
    // }
    // stamp.Log() << "common end" << endl;

    stamp.Log() << "global start" << endl;
    for (size_t i = 0; i < N; i++)
        globalTest();
    stamp.Log() << "global end" << endl;
}

int main(int argc, char *argv[])
{
    SingletonCLI::Initialize();
    CommonLogger::Initialize("./test/test.conf");

    SingletonCLI::GetInstance().GetCLI()->callback(unittest_CommonLogger);

    SingletonCLI::GetInstance().Parse(argc, argv, {"./test/test.conf"});
}