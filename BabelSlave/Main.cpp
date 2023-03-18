#include "Application.hpp"
#include <filesystem>
#include "Core/Logger.hpp"
#include "AoResources/Resources.hpp"

int main(int argc, char* argv[])
{
    if (argc < 4) return -1;
    try
    {
        Babel::AppSettings settings;
        Babel::LOGGER->init("Logs/BabelUI.log", "BabelUI");
        settings.Width = std::atoi(argv[1]);
        settings.Height = std::atoi(argv[2]);
        settings.CompressedResources = std::atoi(argv[3]) > 0;
        settings.EnableDebug = std::atoi(argv[4]) > 0;
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