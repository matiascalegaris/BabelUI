#pragma once
#include <string>

std::string Encode(const std::string source, const std::string password);
std::string Decode(const std::string source, const std::string password);
