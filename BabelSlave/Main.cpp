#include "Application.hpp"
#include <filesystem>

int main(int argc, char* argv[])
{
    if (argc < 3) return -1;
    int width = std::atoi(argv[1]);
    int height = std::atoi(argv[2]);;
    Babel::Application app;
    app.Initialize(width, height);
    app.Run();
	return 0;
}