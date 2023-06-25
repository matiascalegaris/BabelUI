#pragma once
#include <Ultralight/platform/FileSystem.h>
#include <Windows.h>
#include <memory>
#include <map>
#include <vector>

namespace AO
{
	class Compressor;
}
namespace Babel {
	
	struct CompressedData
	{
		int Id;
		std::vector<uint8_t> data;
	};
	
	class BabelFileSystemWin : public ultralight::FileSystem {
	public:
		// Construct FileSystemWin instance.
		// 
		// @note You can pass a valid baseDir here which will be prepended to
		//       all file paths. This is useful for making all File URLs relative
		//       to your HTML asset directory.
		BabelFileSystemWin(LPCWSTR baseDir, bool compressedResources);

		virtual ~BabelFileSystemWin();

		virtual bool FileExists(const ultralight::String& file_path) override;

		virtual ultralight::String GetFileMimeType(const ultralight::String& file_path) override;

		virtual ultralight::String GetFileCharset(const ultralight::String& file_path) override;

		virtual ultralight::RefPtr<ultralight::Buffer> OpenFile(const ultralight::String& file_path) override;
		void ReleaseDecompressedId(int id);
		
	private:
		std::unique_ptr<WCHAR[]> GetRelative(const ultralight::String& path);
		std::unique_ptr<WCHAR[]> baseDir_;
		bool mCompressedResources = {0};
		std::unique_ptr<AO::Compressor> mCompressedGraphics;
		std::unique_ptr<AO::Compressor> mCompressedInit;
		std::unique_ptr<AO::Compressor> mCompressedMiniMaps;
		int mNextIndex = { 0 };
		std::map<int, std::unique_ptr<CompressedData>> mOpenFileMap;
	};
}