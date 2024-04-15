#pragma once
#include <string>
#include <vector>
#include <memory>

namespace AO
{
	class CompressedFile;
	class Compressor
	{
	public:
		Compressor();
		~Compressor();
		void Open(const char* fileName);
		void GetFileData(const char* fileName, std::vector<uint8_t>& data);
		bool HasFile(const char* fileName);
	private:
		std::unique_ptr<CompressedFile> mCompressedFile;
	};
}