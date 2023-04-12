#include "FileUtils.h"
#include <Windows.h>
#include "Core/Logger.hpp"


std::string CallerPath()
{
    static std::string path;
    if (path.empty())
    {
        path = GetCaller();
        size_t lastSlashPos = path.find_last_of("\\/");
        if (lastSlashPos != std::string::npos) {
            path = path.substr(0, lastSlashPos);
        }
    }
    return path;
}

std::filesystem::path GetWorkingPath()
{

    auto path = CallerPath();
    //auto path = std::filesystem::path("D:\\Proyectos/ao20/Recursos");
    path += "/../Recursos/";
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

std::string GetCaller()
{
    char szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH);
    return std::string(szPath);
}
