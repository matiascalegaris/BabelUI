#pragma once
#include <filesystem>
#include <string>

std::filesystem::path GetWorkingPath();
std::filesystem::path GetFilePath(const char* fileName);
std::wstring LocalPathForFile(const char* fileName);

std::string GetCaller();
std::string CallerPath();


