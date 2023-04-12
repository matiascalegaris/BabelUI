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
#include "AoResources/Compresor.hpp"
#include "Utils//FileUtils.h"

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

    std::wstring GetMimeType(const std::wstring& szExtension) 
    {
        // return mime type for extension
        HKEY hKey = NULL;
        std::wstring szResult = L"application/unknown";

        // open registry key
        if (RegOpenKeyExW(HKEY_CLASSES_ROOT, szExtension.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            // define buffer
            wchar_t szBuffer[256] = { 0 };
            DWORD dwBuffSize = sizeof(szBuffer);

            // get content type
            if (RegQueryValueExW(hKey, L"Content Type", NULL, NULL, (LPBYTE)szBuffer, &dwBuffSize)
                == ERROR_SUCCESS) {
                // success
                szResult = szBuffer;
            }

            // close key
            RegCloseKey(hKey);
        }

        // return result
        return szResult;
    }
    BabelFileSystemWin* sActiveFS = nullptr; 

    BabelFileSystemWin::BabelFileSystemWin(LPCWSTR baseDir, bool compressedResources) : mCompressedResources(compressedResources) 
    {
        baseDir_.reset(new WCHAR[_MAX_PATH]);
        StringCchCopyW(baseDir_.get(), MAX_PATH, baseDir);
        if (compressedResources)
        {
            mCompressedGraphics = std::make_unique<AO::Compressor>();
            mCompressedGraphics->Open(GetFilePath("OUTPUT/Graficos").u8string().c_str(),
                                      "ht5PutasTdyRk6BSJcucumelo234583013lalivn2FRjYYBzPhnMrkmUfLMgm4TDX");
            mCompressedInit = std::make_unique<AO::Compressor>();
            mCompressedInit->Open(GetFilePath("OUTPUT/init").u8string().c_str(),
                "ht5PutasTdyRk6BSJcucumelo234583013lalivn2FRjYYBzPhnMrkmUfLMgm4TDX");
        }
        sActiveFS = this;
    }

    BabelFileSystemWin::~BabelFileSystemWin() { }

    bool BabelFileSystemWin::FileExists(const String& path) 
    {
        WIN32_FIND_DATAW findData;
        if (mCompressedGraphics)
        {
            std::string file = path.utf8().data();
            if (file.rfind("/Graficos/", 0) == 0) {
                auto start = file.rfind("/") + 1;
                file = file.substr(start);
                return mCompressedGraphics->HasFile(file.c_str());
            }
            if (file.rfind("/init/", 0) == 0) {
                auto start = file.rfind("/") + 1;
                file = file.substr(start);
                return mCompressedInit->HasFile(file.c_str());
            }
        }
        return getFindData(GetRelative(path).get(), findData);
    }

    String BabelFileSystemWin::GetFileMimeType(const String& file_path) 
    {
        String16 path16 = file_path.utf16();
        LPWSTR ext = PathFindExtensionW(path16.data());
        std::wstring mimetype = GetMimeType(ext);
        return String16(mimetype.c_str(), mimetype.length());
    }

    String BabelFileSystemWin::GetFileCharset(const String& file_path) { return "utf-8"; }

    struct FileSystemWin_BufferData 
    {
        HANDLE hFile;
        HANDLE hMap;
        LPVOID lpBasePtr;
    };

    void FileSystemWin_DestroyBufferCallback(void* user_data, void* data) 
    {
        FileSystemWin_BufferData* buffer_data = reinterpret_cast<FileSystemWin_BufferData*>(user_data);
        UnmapViewOfFile(buffer_data->lpBasePtr);
        CloseHandle(buffer_data->hMap);
        CloseHandle(buffer_data->hFile);
        delete buffer_data;
    }

    void FileSystemWin_DestroyDecompressedFileData(void* user_data, void* data)
    {
        CompressedData* comppresedData = reinterpret_cast<CompressedData*>(user_data);
        sActiveFS->ReleaseDecompressedId(comppresedData->Id);
    }

    RefPtr<Buffer> BabelFileSystemWin::OpenFile(const String& file_path) 
    {
        auto pathStr = GetRelative(file_path);
        HANDLE hFile;
        HANDLE hMap;
        LPVOID lpBasePtr;
        LARGE_INTEGER liFileSize;
        if (mCompressedGraphics)
        {
            std::string file = file_path.utf8().data();
            if (file.rfind("/Graficos/", 0) == 0) {
                auto start = file.rfind("/") + 1;
                file = file.substr(start);                
                mNextIndex++;
                auto it = mOpenFileMap.insert(std::make_pair(mNextIndex, std::make_unique<CompressedData>()));
                it.first->second->Id = mNextIndex;
                mCompressedGraphics->GetFileData(file.c_str(), it.first->second->data);
                return Buffer::Create(it.first->second->data.data(), it.first->second->data.size(), it.first->second.get(),
                    FileSystemWin_DestroyDecompressedFileData);
            }
            if (file.rfind("/init/", 0) == 0) {
                auto start = file.rfind("/") + 1;
                file = file.substr(start);
                mNextIndex++;
                auto it = mOpenFileMap.insert(std::make_pair(mNextIndex, std::make_unique<CompressedData>()));
                it.first->second->Id = mNextIndex;
                mCompressedInit->GetFileData(file.c_str(), it.first->second->data);
                return Buffer::Create(it.first->second->data.data(), it.first->second->data.size(), it.first->second.get(),
                    FileSystemWin_DestroyDecompressedFileData);
            }
        }
        hFile = CreateFileW(pathStr.get(),
            GENERIC_READ,          // dwDesiredAccess
            FILE_SHARE_READ,       // dwShareMode
            NULL,                  // lpSecurityAttributes
            OPEN_EXISTING,         // dwCreationDisposition
            FILE_ATTRIBUTE_NORMAL, // dwFlagsAndAttributes
            0);                    // hTemplateFile

        if (hFile == INVALID_HANDLE_VALUE) 
        {
            return nullptr;
        }

        if (!GetFileSizeEx(hFile, &liFileSize)) 
        {
            CloseHandle(hFile);
            return nullptr;
        }

        if (liFileSize.QuadPart == 0) 
        {
            CloseHandle(hFile);
            return nullptr;
        }

        hMap = CreateFileMapping(hFile,
            NULL,          // Mapping attributes
            PAGE_READONLY, // Protection flags
            0,             // MaximumSizeHigh
            0,             // MaximumSizeLow
            NULL);         // Name

        if (hMap == 0) 
        {
            CloseHandle(hFile);
            return nullptr;
        }

        lpBasePtr = MapViewOfFile(hMap,
            FILE_MAP_READ, // dwDesiredAccess
            0,             // dwFileOffsetHigh
            0,             // dwFileOffsetLow
            0);            // dwNumberOfBytesToMap

        if (lpBasePtr == NULL) 
        {
            CloseHandle(hMap);
            CloseHandle(hFile);
            return nullptr;
        }

        FileSystemWin_BufferData* buffer_data = new FileSystemWin_BufferData();
        buffer_data->hFile = hFile;
        buffer_data->hMap = hMap;
        buffer_data->lpBasePtr = lpBasePtr;

        return Buffer::Create((char*)lpBasePtr, (size_t)liFileSize.QuadPart, buffer_data,
            FileSystemWin_DestroyBufferCallback);
    }

    void BabelFileSystemWin::ReleaseDecompressedId(int id)
    {
        mOpenFileMap.erase(id);
    }

    std::unique_ptr<WCHAR[]> BabelFileSystemWin::GetRelative(const String& path) 
    {
        String16 path16 = path.utf16();
        std::unique_ptr<WCHAR[]> relPath(new WCHAR[_MAX_PATH]);
        memset(relPath.get(), 0, _MAX_PATH * sizeof(WCHAR));
        PathCombineW(relPath.get(), baseDir_.get(), path16.data());
        return relPath;
    }

} 