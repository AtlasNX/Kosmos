#ifndef PACKAGE_H
#define PACKAGE_H
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include <string>

#if defined(SWITCH)
#define ROOT_PATH "/"
#elif defined(__WIIU__)
#define ROOT_PATH "fs:/vol/external01/"
#else
#define ROOT_PATH "sdroot/"
#endif

class Package
{
public:
	Package(int state);
	~Package();

	std::string toString();
	bool downloadZip(const char* tmp_path, float* progress = NULL);
	bool install(const char* pkg_path, const char* tmp_path);
	bool remove(const char* pkg_path);
	const char* statusString();
	void updateStatus(const char* pkg_path);

	int isPreviouslyInstalled();

	// Package attributes
	std::string pkg_name;
	std::string title;
	std::string author;
	std::string short_desc;
	std::string long_desc;
	std::string version;

	std::string license;
	std::string changelog;
	std::string url;
	std::string updated;
	std::string binary;
	int updated_timestamp = 0;

	int downloads = 0;
	int extracted_size = 0;
	int download_size = 0;

	std::string category;

	// Sorting attributes
	//	  Repo* parentRepo;
	std::string* repoUrl;

	int status; // local, update, installed, get

	// bitmask for permissions, from left to right:
	// unused, iosu, kernel, nand, usb, sd, wifi, sound
	char permissions;

	// the downloaded contents file, to keep memory around to cleanup later
	std::string* contents;
};

#endif
