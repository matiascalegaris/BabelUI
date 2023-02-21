#include "FileUtils.h"

std::filesystem::path GetWorkingPath()
{
    //auto path = std::filesystem::current_path();
    auto path = std::filesystem::path("D:\\Proyectos/ao20/Recursos");
    path += "/../Recursos/BabelUI/";
    return path;
}

std::filesystem::path GetFilePath(const char* fileName)
{
    auto path = GetWorkingPath();
    path += fileName;
    return path;
}

std::wstring LocalPathForFile(const char* fileName)
{
    auto path = GetFilePath(fileName).native();
    return std::wstring(L"file:///") + path;
}