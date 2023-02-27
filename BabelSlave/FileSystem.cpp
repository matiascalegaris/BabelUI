#ifndef NOMINMAX
# define NOMINMAX
#endif
#include "FileSystem.h"
#include <io.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <sys/stat.h>
#include <windows.h>
#include <limits>
#include <stdio.h>
#include <Ultralight/String.h>
#include <string>
#include <algorithm>
#include <memory>
#include <Strsafe.h>
#include <filesystem>

namespace Babel {
    using namespace ultralight;
    static bool getFindData(LPCWSTR path, WIN32_FIND_DATAW& findData)
    {
        HANDLE handle = FindFirstFileW(path, &findData);
        if (handle == INVALID_HANDLE_VALUE)
            return false;
        FindClose(handle);
        return true;
    }

    static bool getFileSizeFromByHandleFileInformationStructure(const BY_HANDLE_FILE_INFORMATION& fileInformation, int64_t& size)
    {
        ULARGE_INTEGER fileSize;
        fileSize.HighPart = fileInformation.nFileSizeHigh;
        fileSize.LowPart = fileInformation.nFileSizeLow;

        if (fileSize.QuadPart > static_cast<ULONGLONG>(std::numeric_limits<int64_t>::max()))
            return false;

        size = fileSize.QuadPart;
        return true;
    }

    bool getFileSize(FileHandle fileHandle, int64_t& size)
    {
        BY_HANDLE_FILE_INFORMATION fileInformation;
        if (!::GetFileInformationByHandle((HANDLE)fileHandle, &fileInformation))
            return false;

        return getFileSizeFromByHandleFileInformationStructure(fileInformation, size);
    }

    std::wstring GetMimeType(const std::wstring& szExtension)
    {
        // return mime type for extension
        HKEY hKey = NULL;
        std::wstring szResult = L"application/unknown";

        // open registry key
        if (RegOpenKeyExW(HKEY_CLASSES_ROOT, szExtension.c_str(),
            0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            // define buffer
            wchar_t szBuffer[256] = { 0 };
            DWORD dwBuffSize = sizeof(szBuffer);

            // get content type
            if (RegQueryValueExW(hKey, L"Content Type", NULL, NULL,
                (LPBYTE)szBuffer, &dwBuffSize) == ERROR_SUCCESS)
            {
                // success
                szResult = szBuffer;
            }

            // close key
            RegCloseKey(hKey);
        }

        // return result
        return szResult;
    }

    BabelFileSystemWin::BabelFileSystemWin(LPCWSTR baseDir) {
        baseDir_.reset(new WCHAR[_MAX_PATH]);
        StringCchCopyW(baseDir_.get(), MAX_PATH, baseDir);
    }

    BabelFileSystemWin::~BabelFileSystemWin() {}

    bool BabelFileSystemWin::FileExists(const String16& path) {
        WIN32_FIND_DATAW findData;
        return getFindData(GetRelative(path).get(), findData);
    }

    bool BabelFileSystemWin::GetFileMimeType(const String16& path, String16& result)
    {
        auto ext = std::filesystem::path(path.data()).extension();
        std::wstring mimetype = GetMimeType(ext);
        result = String16(mimetype.c_str(), mimetype.length());
        return true;
    }

    bool BabelFileSystemWin::GetFileSize(FileHandle handle, int64_t& result) {
        BY_HANDLE_FILE_INFORMATION fileInformation;
        if (!::GetFileInformationByHandle((HANDLE)handle, &fileInformation))
            return false;

        return getFileSizeFromByHandleFileInformationStructure(fileInformation, result);
    }

    FileHandle BabelFileSystemWin::OpenFile(const String16& path, bool open_for_writing) {
        DWORD desiredAccess = 0;
        DWORD creationDisposition = 0;
        DWORD shareMode = 0;
        if (open_for_writing) {
            desiredAccess = GENERIC_WRITE;
            creationDisposition = CREATE_ALWAYS;
        }
        else {
            desiredAccess = GENERIC_READ;
            creationDisposition = OPEN_EXISTING;
            shareMode = FILE_SHARE_READ;
        }

        return (FileHandle)CreateFileW(GetRelative(path).get(), desiredAccess, shareMode,
            0, creationDisposition, FILE_ATTRIBUTE_NORMAL, 0);
    }

    void BabelFileSystemWin::CloseFile(FileHandle& handle) {
        if (handle != (FileHandle)INVALID_HANDLE_VALUE) {
            ::CloseHandle((HANDLE)handle);
            handle = (FileHandle)INVALID_HANDLE_VALUE;
        }
    }

    int64_t BabelFileSystemWin::ReadFromFile(FileHandle handle, char* data, int64_t length) {
        if (handle == (FileHandle)INVALID_HANDLE_VALUE)
            return -1;

        DWORD bytesRead;
        bool success = !!::ReadFile((HANDLE)handle, data, (DWORD)length, &bytesRead, 0);

        if (!success)
            return -1;
        return static_cast<int64_t>(bytesRead);
    }

    std::unique_ptr<WCHAR[]> BabelFileSystemWin::GetRelative(const String16& path) {
        std::unique_ptr<WCHAR[]> relPath(new WCHAR[_MAX_PATH]);
        memset(relPath.get(), 0, _MAX_PATH * sizeof(WCHAR));

        // Get the absolute paths of the input and base directories
        std::filesystem::path absBaseDir = std::filesystem::absolute(baseDir_.get());
        absBaseDir += path.data();

        // Convert the path to a null-terminated wide string
        std::wstring relPathStr = absBaseDir.native();
        wcscpy_s(relPath.get(), relPathStr.size() + 1, relPathStr.c_str());
        return relPath;
    }

}  // namespace ultralight
