#include "Application.hpp"
#include <filesystem>
#include "Core/Logger.hpp"
#include "AoResources/Resources.hpp"
#include "Utils/FileUtils.h"
#include <csignal>
#include <windows.h>
#include "resource.h"

void signalHandler(int signum) {
    

    // cleanup and close up stuff here  
    // terminate program  
    switch (signum)
    {
    case SIGABRT:
        Babel::LOGGER->log("Got error signal SIGABRT");
        break;
    case SIGFPE:
        Babel::LOGGER->log("Got error signal SIGFPE");
        break;
    case SIGILL:
        Babel::LOGGER->log("Got error signal SIGILL");
        break;
    case SIGSEGV:
        Babel::LOGGER->log("Got error signal SIGSEGV");
        break;
    case SIGINT:
        Babel::LOGGER->log("Got error signal SIGINT");
        break;
    default:
        Babel::LOGGER->log("Got error signal " + std::to_string(signum));
        break;
    }
}

void TestEmbebed()
{
    
}

int main(int argc, char* argv[])
{
    if (argc < 6) return -1;
    try
    {
        signal(SIGABRT, signalHandler);
        signal(SIGFPE, signalHandler);
        signal(SIGILL, signalHandler);
        signal(SIGSEGV, signalHandler);
        TestEmbebed();
        Babel::AppSettings settings;
        Babel::LOGGER->init(CallerPath() + "/Logs/BabelUI.log", "BabelUI");
        Babel::LOGGER->log("Slave initialize");
        settings.Width = std::atoi(argv[1]);
        settings.Height = std::atoi(argv[2]);
        settings.CompressedResources = std::atoi(argv[3]) > 0;
        settings.EnableDebug = std::atoi(argv[4]) > 0;
        settings.ParentProcessId = std::atoi(argv[5]);
        settings.TunnelName = argv[6];
        Babel::Application app;
        app.Initialize(settings);
        app.Run();
        Babel::LOGGER->log("Normal Exit");

    }
    catch (std::exception& err)
    {
        auto error = err.what();
        Babel::LOGGER->log(error);
    }
    signal(SIGABRT, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
	return 0;
}