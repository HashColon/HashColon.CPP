#include <iostream>
#include <chrono>
#include <HashColon/Helper.hpp>
#include <HashColon/Log.hpp>
#include <HashColon/SingletonCLI.hpp>

using namespace std;
using namespace std::chrono;
using namespace HashColon;

#include <sys/resource.h>
void test_CPUMEM()
{
    rusage testusage;
    int re = getrusage(RUSAGE_SELF, &testusage);

    cout << "shit!" << endl;

    cout << "utime : " << testusage.ru_utime.tv_usec << endl;
    cout << "stime : " << testusage.ru_stime.tv_usec << endl;
    cout << "maxrss: " << testusage.ru_maxrss << endl;
}

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
    int pid = ::getpid();
    cout << "f1: " << pid << endl;
    auto re1 = GetCpuMem_fromPID(pid);
    cout << "re1: cpu%: " << re1.first << ", mem%: " << re1.second << endl;

    size_t N = 100000;

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

    stamp.Log() << "common start" << endl;
    for (size_t i = 0; i < N; i++)
    {
        CommonLogger log2(params_local);
        for (size_t j = 0; j < 10; j++)
        {
            log2.Log() << "AAAA" << endl;
        }
    }
    stamp.Log() << "common end" << endl;

    stamp.Log() << "global start" << endl;
    for (size_t i = 0; i < N; i++)
        globalTest();
    stamp.Log() << "global end" << endl;

    auto re2 = GetCpuMem_fromPID(pid);
    cout << "re2: cpu%: " << re2.first << ", mem%: " << re2.second << endl;
}

int main(int argc, char *argv[])
{
    // SingletonCLI::Initialize();
    // CommonLogger::Initialize("./test/test.conf", "Log");

    // SingletonCLI::GetInstance().GetCLI()->callback(unittest_CommonLogger);

    // // SingletonCLI::GetInstance().GetCLI()->callback();

    // SingletonCLI::GetInstance().Parse(argc, argv, {"./test/test.conf"});

    TimePoint a("2022-12-12 13:00:00");

    cout << TimePoint::Now().Local2Utc() << endl;

    cout << TimePoint::Now().toString() << endl;
    cout << TimePoint::UtcNow().toString() << endl;
}