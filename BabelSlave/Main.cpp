#include "Application.hpp"
#include <filesystem>
#include "Core/Logger.hpp"
#include "AoResources/Resources.hpp"
int main(int argc, char* argv[])
{
    if (argc < 3) return -1;
    try
    {
        Babel::LOGGER->init("Logs/BabelUI.log", "BabelUI");
        int width = std::atoi(argv[1]);
        int height = std::atoi(argv[2]);;
        Babel::Application app;
        app.Initialize(width, height);
        app.Run();
    }
    catch (std::exception& err)
    {
        auto error = err.what();
    }
    
	return 0;
}