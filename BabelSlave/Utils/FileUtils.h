#pragma once
#include <filesystem>
#include <string>

std::filesystem::path GetWorkingPath();
std::filesystem::path GetFilePath(const char* fileName);
std::wstring LocalPathForFile(const char* fileName);

std::string GetCaller();
std::string CallerPath();

std::string ReadPasswordFromAOBin() {
    // Directly specify the path to the AO.bin file
    std::wstring filePath = GetFilePath("OUTPUT/AO.bin").u8string();
    
    // Open the file in binary mode at the end to get the file size
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file\n";
        return "";
    }

    // Get the size of the file
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg); // Move back to the start of the file

    // Read the file into a buffer
    std::vector<unsigned char> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cerr << "Failed to read file\n";
        return "";
    }

    // Calculate the password length from the last two bytes of the buffer
    int length = buffer.back() + buffer[buffer.size() - 2] * 256;

    // Construct the password
    std::string password;
    for (int i = 0; i < length; ++i) {
        password += static_cast<char>(buffer[i * 3 - 1] ^ 37);
    }

    return password;
}
