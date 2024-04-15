#include "Compresor.hpp"
#include <cstdint> // For int32_t
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "Utils/md5.h"
#include "Utils/StringUtils.hpp"
#include "zlib.h"
#include "tomcrypt.h"
#include <algorithm>

namespace AO {
namespace {

	std::string GetPasswordFromAOBin()
	{
		// Directly specify the path to the AO.bin file
		auto filePath = GetFilePath("OUTPUT/AO.bin").u8string();

		// Open the file in binary mode at the end to get the file size
		std::ifstream file(filePath, std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			Babel::LOGGER->log("Failed to open AO.bin");
			return "";
		}

		// Get the size of the file
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg); // Move back to the start of the file

		// Read the file into a buffer
		std::vector<unsigned char> buffer(size);
		if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
			Babel::LOGGER->log("Failed to read AO.bin");
			return "";
		}

		// Calculate the password length from the last two bytes of the buffer
		int length = buffer.back() + buffer[buffer.size() - 2] * 256;

		// Construct the password
		std::string passwordTemp;
		for (int i = 0; i < length; ++i) {
			passwordTemp += static_cast<char>(buffer[i * 3 - 1] ^ 37);
		}

		return passwordTemp;
	}

	void DoCrypt_Data(std::vector<uint8_t>& data, const std::string& password)
	{
		int i, c;
		// Recorro todos los bytes haciendo Xor con la contraseña, variando también el caracter elegido de la contraseña

		c = (data.size() % password.length() - 1);
		if (c < 0) c = password.length() + c;
		for (i = 0; i < data.size(); i++)
		{
			data[i] = data[i] ^ (password[c] & 0xFF);
			c--;
			if (c < 0) c = (password.length() -1);
		}
	}

	bool DecrypAes(std::vector<uint8_t>& data)
	{
		std::vector<uint8_t> output;
		output.resize(data.size());
		if (register_cipher(&aes_desc) == -1) {
			return false;
		}
		unsigned char key[16] = { 0xAB, 0x45, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xA0, 0xE1, 0xD2, 0xC3, 0xB4, 0xA5, 0x96, 0x66 };
		unsigned char iv[16] = { 0xFE, 0xDC, 0xAA, 0x9A, 0x76, 0x54, 0x3A, 0x10, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x33, 0x22 };
		symmetric_CFB cfb;
		int err;

		/* Initialize CFB mode */
		if ((err = cfb_start(find_cipher("aes"), iv, key, 16, 0, &cfb)) != CRYPT_OK) {
			return false;
		}

		/* Decrypt the ciphertext */
		if ((err = cfb_decrypt(data.data(), output.data(), data.size(), &cfb)) != CRYPT_OK) {
			return false;
		}
		cfb_done(&cfb);
		data = output;
		return true;
	}
}
#pragma pack(push, 1) // Set struct packing to 1 byte alignment
	struct INFOHEADER {
		int32_t lngFileStart;                   // Where does the chunk start?
		int32_t lngFileSize;                    // How big is this chunk of stored data?
		char strFileName[32];                   // What's the name of the file this data came from?
		int32_t lngFileSizeUncompressed;        // How big is the file compressed?
	};


	struct FILEHEADER {
		int32_t lngFileSize;                 // How big is this file? (Used to check integrity)
		int16_t intNumFiles;                 // How many files are inside?
		char lngPassword[32];                // Integrity check (string of length 32 bytes)
	};
#pragma pack(pop) // Restore original packing

	class CompressedFile 
	{
	public:
		~CompressedFile() {}
		void Open(const char* fileName, const std::string& password);
		bool GetFileData(std::string fileName, std::vector<uint8_t>& data);
		bool HasFile(std::string fileName);
	private:
		std::ifstream mFile;
		FILEHEADER mFileHeader;
		std::map<std::string, INFOHEADER> mFileMap;
		std::string mPassword;
	};

	Compressor::Compressor()
	{
	}

	Compressor::~Compressor()
	{
	}

	void Compressor::Open(const char* fileName)
	{
		std::string password = GetPasswordFromAOBin();
		mCompressedFile = std::make_unique<CompressedFile>();
		mCompressedFile->Open(fileName, password);
	}

	void Compressor::GetFileData(const char* fileName, std::vector<uint8_t>& data)
	{
		mCompressedFile->GetFileData(fileName, data);
	}

	bool Compressor::HasFile(const char* fileName)
	{
		return mCompressedFile->HasFile(fileName);
	}

	void CompressedFile::Open(const char* fileName, const std::string& password)
	{
		if (mFile.is_open())
		{
			return;
		}
		mFile.open(fileName, std::ios::in | std::ios::binary);
		if (!mFile.is_open())
		{
			return;
		}
		// header
		mFile.read(reinterpret_cast<char*>(&mFileHeader), sizeof(mFileHeader));
		if (password.empty())
		{
			mPassword = "Contraseña";
		}
		else
		{
			mPassword = password;
		}

		auto pwdHash = md5(mPassword);
		if (pwdHash != mFileHeader.lngPassword)
		{
			return;
		}
		int min = 1;
		int max = mFileHeader.intNumFiles;
		for (int i = 0; i < mFileHeader.intNumFiles; i++)
		{
			INFOHEADER fileInfo;
			mFile.read(reinterpret_cast<char*>(&fileInfo), sizeof(fileInfo));
			std::string fName(fileInfo.strFileName, 32);
			rtrim(fName);
			mFileMap.insert(std::make_pair(fName, fileInfo));
		}
	}

	bool CompressedFile::GetFileData(std::string fileName, std::vector<uint8_t>& data)
	{
		std::transform(fileName.begin(), fileName.end(), fileName.begin(),
			[](unsigned char c) { return std::tolower(c); });
		auto it = mFileMap.find(fileName);
		if (it == mFileMap.end()) return false;

		mFile.seekg(it->second.lngFileStart-1);
		std::vector<uint8_t> compressedData;
		compressedData.resize(it->second.lngFileSize);
		mFile.read(reinterpret_cast<char*>(compressedData.data()), compressedData.size());
		DoCrypt_Data(compressedData, mPassword);
		data.resize(it->second.lngFileSizeUncompressed);
		DecrypAes(compressedData);
		uLongf size = data.size();
		auto ret = uncompress(data.data(), &size, compressedData.data(), compressedData.size());
		return true;
	}

	bool CompressedFile::HasFile(std::string fileName)
	{
		std::transform(fileName.begin(), fileName.end(), fileName.begin(),
			[](unsigned char c) { return std::tolower(c); });
		auto it = mFileMap.find(fileName);
		if (it == mFileMap.end()) return false;
		return true;
	}
}