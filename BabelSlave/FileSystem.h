#pragma once
#include <Ultralight/platform/FileSystem.h>
#include <Windows.h>
#include <memory>

namespace Babel {
	class BabelFileSystemWin : public ultralight::FileSystem {
	public:
		// Construct FileSystemWin instance.
		// 
		// @note You can pass a valid baseDir here which will be prepended to
		//       all file paths. This is useful for making all File URLs relative
		//       to your HTML asset directory.
		BabelFileSystemWin(LPCWSTR baseDir);

		virtual ~BabelFileSystemWin();

		virtual bool FileExists(const ultralight::String16& path) override;

		virtual bool GetFileSize(ultralight::FileHandle handle, int64_t& result) override;

		virtual bool GetFileMimeType(const ultralight::String16& path, ultralight::String16& result) override;

		virtual ultralight::FileHandle OpenFile(const ultralight::String16& path, bool open_for_writing) override;

		virtual void CloseFile(ultralight::FileHandle& handle) override;

		virtual int64_t ReadFromFile(ultralight::FileHandle handle, char* data, int64_t length) override;

	protected:
		std::unique_ptr<WCHAR[]> GetRelative(const ultralight::String16& path);

		std::unique_ptr<WCHAR[]> baseDir_;
	};
}