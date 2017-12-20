#include <fstream>
#include <thread>

#include <boost/asio.hpp>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include <assistx2/platform_wrapper.h>

#include "ConfigMgr.h"
#include "RunFastGameMgr.h"
#include "helper.h"
#include "DataLayer.h"
#include "version.h"
#include "datemanager.h"

static std::string pidfile_name;

DEFINE_string(cfg, "",  "cfg file path.");

static bool ValidateCfg(const char * flagname, const std::string & value)
{
    std::fstream file(value.c_str(), std::ios::in);
    if (file.is_open() == false)
    {
        std::cout<<"read cfg:="<<value<<", failed."<<std::endl;
        return false;
    }
    else
    {
        return true;
    }
}

#if !_WIN32

/* for safely exit, make sure to do checkpoint*/
static void sig_handler(const int sig)
{
    LOG(INFO) << "Signal:" << sig << " SIGUSR1:" << SIGUSR1;
    if (sig == SIGUSR1)
    {

        return;
    }

    if (sig != SIGTERM && sig != SIGQUIT && sig != SIGINT && sig != SIGKILL) {
        return;
    }

    LOG(ERROR)<<"Signal "<<sig<<" handled, niuniu is now exit..";

    unlink(pidfile_name.c_str());

    /* make sure deadlock detect loop is quit*/
    sleep(2);

    /* then we exit to call axexit */
    exit(EXIT_SUCCESS);
}

#endif

void SignalHandle(const char* data, int size)
{
    std::string str = std::string(data, size);
    LOG(ERROR) << str;
}

int main(int argc,  char ** argv)
{
    google::RegisterFlagValidator(&FLAGS_cfg, &ValidateCfg);

    google::ParseCommandLineFlags(&argc, &argv, true);

    google::InitGoogleLogging(argv[0]);

    google::InstallFailureSignalHandler();
    google::InstallFailureWriter(&SignalHandle);

    LOG(INFO)<<"XPDKPOKER_VERSION:="<< XPDKPOKER_VERSION;

    srand(static_cast<unsigned int>(time(nullptr) )  );

    boost::asio::io_service ios;

    if (ConfigMgr::getInstance()->LoadLocalCfg() != 0)
    {
        LOG(INFO)<<"LoadLocalCfg Failed";
        return 0;
    }

    if (DataLayer::getInstance()->Init(&ios) != 0)
    {
        LOG(ERROR)<<"DataLayer init Failed";
        return 0;
    }

 #if !_WIN32
    if (signal(SIGTERM, sig_handler) == SIG_ERR)
        fprintf(stderr, "can not catch SIGTERM");
    if (signal(SIGQUIT, sig_handler) == SIG_ERR)
        fprintf(stderr, "can not catch SIGQUIT");
    if (signal(SIGINT,  sig_handler) == SIG_ERR)
        fprintf(stderr, "can not catch SIGINT");
    if (signal(SIGUSR1, sig_handler) == SIG_ERR)
        fprintf(stderr, "can not catch SIGUSR1");
 #endif

    char path[512] = {0};

    std::fstream pid;
    pid.open(ConfigMgr::getInstance()->GetPidFilePath(), std::fstream::trunc | std::fstream::out);
    pid << assistx2::os::GetCurrentDirectory(path, 511) << " " << assistx2::os::getpid();
    pid.close();

    RunFastGameMgr mgr(ios);
    mgr.Initialize();

    DataManager::getInstance()->Init();

    boost::system::error_code ec;
    ios.run(ec);

    mgr.ShutDown();

    DataLayer::Destroy();

    ConfigMgr::Destroy();

    google::ShutdownGoogleLogging();

    google::ShutDownCommandLineFlags();

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
