#include "Application.hpp"
#include <filesystem>
#include "Core/Logger.hpp"
#include "AoResources/Resources.hpp"
#include "Utils/FileUtils.h"

int main(int argc, char* argv[])
{
    if (argc < 6) return -1;
    try
    {
        Babel::AppSettings settings;
        Babel::LOGGER->init(CallerPath() + "Logs/BabelUI.log", "BabelUI");
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
    }
    catch (std::exception& err)
    {
        auto error = err.what();
    }
    
	return 0;
}