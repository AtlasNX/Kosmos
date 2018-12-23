// Modified from Zarklord1 : https://github.com/Zarklord1/WiiU_Save_Manager (src/file_utils/Zip.cpp)
#include <cstdint>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unistd.h>

#include "Utils.hpp"
#include "ZipUtil.hpp"

#define u32 uint32_t
#define u8 uint8_t

using namespace std;

Zip::Zip(const char* zipPath)
{
	fileToZip = zipOpen(zipPath, APPEND_STATUS_CREATE);
	if (fileToZip == NULL) printf("Error Opening: %s for zipping files!\n", zipPath);
}

Zip::~Zip()
{
	Close();
}

int Zip::AddFile(const char* internalPath, const char* path)
{
	zipOpenNewFileInZip(fileToZip, internalPath, NULL, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
	printf("Adding %s to zip, under path %s\n", path, internalPath);
	int code = Add(path);
	zipCloseFileInZip(fileToZip);
	return code;
}

int Zip::AddDir(const char* internalDir, const char* externalDir)
{
	struct dirent* dirent = NULL;
	DIR* dir = NULL;

	dir = opendir(externalDir);
	if (dir == NULL)
	{
		return -1;
	}

	while ((dirent = readdir(dir)) != 0)
	{
		if (strcmp(dirent->d_name, "..") == 0 || strcmp(dirent->d_name, ".") == 0)
			continue;

		std::string zipPath(internalDir);
		zipPath += '/';
		zipPath += dirent->d_name;
		std::string realPath(externalDir);
		realPath += '/';
		realPath += dirent->d_name;

		if (dirent->d_type & DT_DIR)
		{
			AddDir(zipPath.c_str(), realPath.c_str());
		}
		else
		{
			AddFile(zipPath.c_str(), realPath.c_str());
		}
	}
	closedir(dir);
	return 0;
}

int Zip::Add(const char* path)
{

	int fileNumber = open(path, O_RDONLY);
	if (fileNumber == -1)
		return -1;

	u32 filesize = lseek(fileNumber, 0, SEEK_END);
	lseek(fileNumber, 0, SEEK_SET);

	u32 blocksize = 0x8000;
	u8* buffer = (u8*)malloc(blocksize);
	if (buffer == NULL)
		return -2;

	u32 done = 0;
	int readBytes = 0;

	while (done < filesize)
	{
		if (done + blocksize > filesize)
		{
			blocksize = filesize - done;
		}
		readBytes = read(fileNumber, buffer, blocksize);
		if (readBytes <= 0)
			break;
		zipWriteInFileInZip(fileToZip, buffer, blocksize);
		done += readBytes;
	}
	close(fileNumber);
	free(buffer);

	if (done != filesize)
	{
		return -3;
	}
	return 0;
}

void Zip::Close()
{
	zipClose(fileToZip, NULL);
}

UnZip::UnZip(const char* zipPath)
{
	fileToUnzip = unzOpen(zipPath);
}

UnZip::~UnZip()
{
	Close();
}

void UnZip::Close()
{
	unzClose(fileToUnzip);
}

int UnZip::ExtractFile(const char* internalPath, const char* path)
{
	int code = unzLocateFile(fileToUnzip, internalPath, 0);
	if (code == UNZ_END_OF_LIST_OF_FILE)
		return -1;

	unz_file_info_s* fileInfo = GetFileInfo();

	std::string fullPath(path);
	printf("Extracting file %s to: %s\n", internalPath, fullPath.c_str());
	code = Extract(fullPath.c_str(), fileInfo);
	free(fileInfo);
	return code;
}

int UnZip::ExtractDir(const char* internalDir, const char* externalDir)
{
	int i = 0;
	for (;;)
	{
		int code;
		if (i == 0)
		{
			code = unzGoToFirstFile(fileToUnzip);
			i++;
		}
		else
		{
			code = unzGoToNextFile(fileToUnzip);
		}
		if (code == UNZ_END_OF_LIST_OF_FILE)
		{
			if (i > 1)
			{
				return 0;
			}
			else
			{
				return -1;
			}
		}
		unz_file_info_s* fileInfo = GetFileInfo();

		std::string outputPath = GetFullFileName(fileInfo);
		if (outputPath.find(internalDir, 0) != 0)
		{
			free(fileInfo);
			continue;
		}

		outputPath.replace(0, strlen(internalDir), externalDir);
		if (fileInfo->uncompressed_size != 0 && fileInfo->compression_method != 0)
		{
			//file
			i++;
			printf("Extracting %s to: %s\n", GetFullFileName(fileInfo).c_str(), outputPath.c_str());
			Extract(outputPath.c_str(), fileInfo);
		}
		free(fileInfo);
	}
}

int UnZip::ExtractAll(const char* dirToExtract)
{
	int i = 0;
	for (;;)
	{
		int code;
		if (i == 0)
		{
			code = unzGoToFirstFile(fileToUnzip);
			i++;
		}
		else
		{
			code = unzGoToNextFile(fileToUnzip);
		}
		if (code == UNZ_END_OF_LIST_OF_FILE) return 0;

		unz_file_info_s* fileInfo = GetFileInfo();
		std::string fileName(dirToExtract);
		fileName += '/';
		fileName += GetFullFileName(fileInfo);
		if (fileInfo->uncompressed_size != 0 && fileInfo->compression_method != 0)
		{
			//file
			printf("Extracting %s to: %s\n", GetFullFileName(fileInfo).c_str(), fileName.c_str());
			Extract(fileName.c_str(), fileInfo);
		}
		free(fileInfo);
	}
}

int UnZip::Extract(const char* path, unz_file_info_s* fileInfo)
{
	//check to make sure filepath or fileInfo isnt null
	if (path == NULL || fileInfo == NULL)
		return -1;

	if (unzOpenCurrentFile(fileToUnzip) != UNZ_OK)
		return -2;

	char folderPath[strlen(path) + 1];
	strcpy(folderPath, path);
	char* pos = strrchr(folderPath, '/');

	if (pos != NULL)
	{
		*pos = '\0';
		CreateSubfolder(folderPath);
	}

	u32 blocksize = 0x8000;
	u8* buffer = (u8*)malloc(blocksize);
	if (buffer == NULL)
		return -3;
	u32 done = 0;
	int writeBytes = 0;

	FILE* fp = fopen(path, "w");

	if (fp == NULL)
	{
		free(buffer);
		return -4;
	}

	while (done < fileInfo->uncompressed_size)
	{
		if (done + blocksize > fileInfo->uncompressed_size)
		{
			blocksize = fileInfo->uncompressed_size - done;
		}
		unzReadCurrentFile(fileToUnzip, buffer, blocksize);
		writeBytes = write(fileno(fp), buffer, blocksize);
		if (writeBytes <= 0)
		{
			break;
		}
		done += writeBytes;
	}

	fflush(fp);
	fclose(fp);

	free(buffer);

	if (done != fileInfo->uncompressed_size)
		return -4;

	unzCloseCurrentFile(fileToUnzip);
	return 0;
}

std::string UnZip::GetFileName(unz_file_info_s* fileInfo)
{
	char fileName[fileInfo->size_filename + 1];
	std::string path;
	strcpy(fileName, GetFullFileName(fileInfo).c_str());
	char* pos = strrchr(fileName, '/');
	if (pos != NULL)
	{
		pos++;
		path = pos;
	}
	else
	{
		path = fileName;
	}
	return path;
}

std::string UnZip::GetFullFileName(unz_file_info_s* fileInfo)
{
	char filePath[fileInfo->size_filename + 1];
	unzGetCurrentFileInfo(fileToUnzip, fileInfo, filePath, fileInfo->size_filename, NULL, 0, NULL, 0);
	filePath[fileInfo->size_filename] = '\0';
	std::string path(filePath);
	path.resize(fileInfo->size_filename);
	return path;
}

unz_file_info_s* UnZip::GetFileInfo()
{
	unz_file_info_s* fileInfo = (unz_file_info_s*)malloc(sizeof(unz_file_info_s));
	unzGetCurrentFileInfo(fileToUnzip, fileInfo, NULL, 0, NULL, 0, NULL, 0);
	return fileInfo;
}
