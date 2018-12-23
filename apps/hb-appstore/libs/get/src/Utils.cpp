// operating system level utilities
// contains directory utils, http utils, and helper methods

#include <algorithm>
#include <cstdint>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include <curl/curl.h>
#include <curl/easy.h>

#if defined(SWITCH)
#include <switch.h>
#endif

#if defined(__WIIU__)
#include <nsysnet/socket.h>
#endif

#include "Utils.hpp"

int (*networking_callback)(void*, double, double, double, double);

// reference to the curl handle so that we can re-use the connection
CURL* curl = NULL;

bool CreateSubfolder(char* cpath)
{
	std::string path(cpath);
	return mkpath(path);
}

// http://stackoverflow.com/a/11366985
bool mkpath(std::string path)
{
	bool bSuccess = false;
	int nRC = ::mkdir(path.c_str(), 0775);
	if (nRC == -1)
	{
		switch (errno)
		{
		case ENOENT:
			//parent didn't exist, try to create it
			if (mkpath(path.substr(0, path.find_last_of('/'))))
				//Now, try to create again.
				bSuccess = 0 == ::mkdir(path.c_str(), 0775);
			else
				bSuccess = false;
			break;
		case EEXIST:
			//Done!
			bSuccess = true;
			break;
		default:
			bSuccess = false;
			break;
		}
	}
	else
		bSuccess = true;
	return bSuccess;
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

// https://gist.github.com/alghanmi/c5d7b761b2c9ab199157
bool downloadFileToMemory(std::string path, std::string* buffer)
{
	CURLcode res;

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, path.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, networking_callback);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);

		res = curl_easy_perform(curl);

		if (*buffer == "" || *buffer == "404" || res != CURLE_OK)
			return false;

		return true;
	}

	return false;
}

bool downloadFileToDisk(std::string remote_path, std::string local_path)
{
	std::string fileContents;
	bool resp = downloadFileToMemory(remote_path, &fileContents);
	if (!resp)
		return false;

	std::ofstream file(local_path);
	file << fileContents;
	file.close();

	return true;
}

const char* plural(int amount)
{
	return (amount == 1) ? "" : "s";
}

const std::string dir_name(std::string file_path)
{
	// turns "/hi/man/thing.txt to /hi/man"
	int pos = file_path.find_last_of("/");

	// no "/" in string, return empty string
	if (pos == std::string::npos)
		return "";

	return file_path.substr(0, pos);
}

// sorting function: put bigger strings at the front
bool compareLen(const std::string& a, const std::string& b)
{
	return (a.size() > b.size());
}

int init_networking()
{
#if defined(SWITCH)
	socketInitializeDefault();
#endif
#if defined(__WIIU__)
	socket_lib_init();
#endif

	curl_global_init(CURL_GLOBAL_ALL);

	// init our curl handle
	curl = curl_easy_init();

	return 1;
}

int deinit_networking()
{
	curl_easy_cleanup(curl);
	return 1;
}

void cp(const char* from, const char* to)
{
	std::ifstream src(from, std::ios::binary);
	std::ofstream dst(to, std::ios::binary);

	dst << src.rdbuf();
}

std::string toLower(const std::string& str)
{
	std::string lower;
	transform(str.begin(), str.end(), std::back_inserter(lower), tolower);
	return lower;
}

int remove_empty_dirs(const char* name, int count)
{
	// from incoming path, recursively ensure all directories are deleted
	// return the number of files remaining (0 if totally erased and successful)

	// based on https://stackoverflow.com/a/8438663

	int starting_count = count;

	DIR* dir;
	struct dirent* entry;

	// already deleted
	if (!(dir = opendir(name)))
		return true;

	// go through files in directory
	while ((entry = readdir(dir)) != NULL)
	{
		if (entry->d_type == DT_DIR)
		{
			char path[1024];
			// skip current dir or parent dir
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;

			// update path so far
			snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);

			// recursively go into this directory too
			count += remove_empty_dirs(path, count);
		}
		else
		{
			// file found, increase file count
			count++;
		}
	}

	// now that we've been through this directory, check if it was an empty directory
	if (count == starting_count)
	{
		// empty, try rmdir (should only remove if empty anyway)
		rmdir(name);
	}

	closedir(dir);

	// return number of files at this level (total count minus starting)
	return count - starting_count;
}
