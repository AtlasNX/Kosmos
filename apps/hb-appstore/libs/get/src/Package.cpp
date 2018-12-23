#include "Package.hpp"
#include "Utils.hpp"
#include "ZipUtil.hpp"
#include "constants.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_set>
#include <vector>
#define u8 uint8_t

#if defined(__WIIU__)
// include xml files for legacy hb app store support
#include "tinyxml.h"
#endif

Package::Package(int state)
{
	this->pkg_name = "?";
	this->title = "???";
	this->author = "Unknown";
	this->version = "0.0.0";
	this->short_desc = "N/A";
	this->long_desc = "N/A";

	this->license = "";
	this->changelog = "";
	this->url = "";
	this->updated = "";
	this->updated_timestamp = 0;

	this->download_size = 0;
	this->extracted_size = 0;
	this->downloads = 0;

	this->category = "_all";
	this->binary = "none";

	this->status = state;
}

Package::~Package()
{
	delete this->contents;
}

std::string Package::toString()
{
	return "[" + this->pkg_name + "] (" + this->version + ") \"" + this->title + "\" - " + this->short_desc;
}

bool Package::downloadZip(const char* tmp_path, float* progress)
{
	// fetch zip file to tmp directory using curl
	printf("--> Downloading %s to %s\n", this->pkg_name.c_str(), tmp_path);
	return downloadFileToDisk(*(this->repoUrl) + "/zips/" + this->pkg_name + ".zip", tmp_path + this->pkg_name + ".zip");
}

bool Package::install(const char* pkg_path, const char* tmp_path)
{
	// assumes that download was called first

	//! Open the Zip file
	UnZip* HomebrewZip = new UnZip((tmp_path + this->pkg_name + ".zip").c_str());

	//! First extract the Manifest
	std::string ManifestPathInternal = "manifest.install";
	std::string ManifestPath = pkg_path + this->pkg_name + "/" + ManifestPathInternal;
	HomebrewZip->ExtractFile(ManifestPathInternal.c_str(), ManifestPath.c_str());

	//! Then extract the info.json file (to know what version we have installed and stuff)
	std::string jsonPathInternal = "info.json";
	std::string jsonPath = pkg_path + this->pkg_name + "/" + jsonPathInternal;
	HomebrewZip->ExtractFile(jsonPathInternal.c_str(), jsonPath.c_str());

	//! Open the Manifest
	std::ifstream ManifestFile;
	ManifestFile.open(ManifestPath.c_str());

	//! Make sure the manifest is present and not empty
	if (ManifestFile.good())
	{
		//! Parse the manifest
		std::string CurrentLine;

		while (std::getline(ManifestFile, CurrentLine))
		{
			char Mode = CurrentLine.at(0);
			std::string Path = CurrentLine.substr(3);
			std::string ExtractPath = ROOT_PATH + Path;

			int resp = 0;
			switch (Mode)
			{
			case 'E':
				//! Simply Extract, with no checks or anything, won't be deleted upon removal
				printf("%s : EXTRACT\n", Path.c_str());
				resp = HomebrewZip->ExtractFile(Path.c_str(), ExtractPath.c_str());
				break;
			case 'U':
				printf("%s : UPDATE\n", Path.c_str());
				resp = HomebrewZip->ExtractFile(Path.c_str(), ExtractPath.c_str());
				break;
			case 'G':
				printf("%s : GET\n", Path.c_str());
				struct stat sbuff;
				if (stat(ExtractPath.c_str(), &sbuff) != 0) //! File doesn't exist, extract
					resp = HomebrewZip->ExtractFile(Path.c_str(), ExtractPath.c_str());
				else
					printf("File already exists, skipping...");
				break;
			default:
				printf("%s : NOP\n", Path.c_str());
				break;
			}

			if (resp < 0)
			{
				printf("--> Some issue happened while extracting! Error: %d\n", resp);
				return false;
			}
		}
	}
	else
	{
		//! Extract the whole zip
		//		printf("No manifest found: extracting the Zip\n");
		//		HomebrewZip->ExtractAll("sdroot/");
		// TODO: generate a manifest here, it's needed for deletion
		printf("No manifest file found (or error writing manifest download)! Refusing to extract.\n");
		return false;
	}

	ManifestFile.close();

	//! Close the Zip file
	delete HomebrewZip;

	//! Delete the Zip file
	std::remove((tmp_path + this->pkg_name + ".zip").c_str());

	return true;
}

bool Package::remove(const char* pkg_path)
{
	// perform an uninstall of the current package, parsing the cached metadata
	std::string ManifestPathInternal = "manifest.install";
	std::string ManifestPath = pkg_path + this->pkg_name + "/" + ManifestPathInternal;

	printf("-> HomebrewManager::Delete\n");
	std::unordered_set<std::string> uniq_folders;

	struct stat sbuff;
	if (stat(ManifestPath.c_str(), &sbuff) != 0) //! There's no manifest
	{
		// there should've been one!
		// TODO: generate a temporary one
		printf("--> ERROR: no manifest found at %s\n", ManifestPath.c_str());
		return false;
	}

	//! Open the manifest normally
	std::ifstream ManifestFile;
	ManifestFile.open(ManifestPath.c_str());

	//! Parse the manifest
	printf("Parsing the Manifest\n");

	std::string CurrentLine;
	while (std::getline(ManifestFile, CurrentLine))
	{
		char Mode = CurrentLine.at(0);
		std::string DeletePath = ROOT_PATH + CurrentLine.substr(3);

		// the current directory
		std::string cur_dir = dir_name(DeletePath);
		uniq_folders.insert(cur_dir);

		switch (Mode)
		{
		case 'U':
			printf("%s : UPDATE\n", DeletePath.c_str());
			printf("Removing %s\n", DeletePath.c_str());
			std::remove(DeletePath.c_str());
			break;
		case 'G':
			printf("%s : GET\n", DeletePath.c_str());
			printf("Removing %s\n", DeletePath.c_str());
			std::remove(DeletePath.c_str());
			break;
		case 'L':
			printf("%s : LOCAL\n", DeletePath.c_str());
			printf("Removing %s\n", DeletePath.c_str());
			std::remove(DeletePath.c_str());
			break;
		default:
			break;
		}
	}

	// sort unique folders from longest to shortest
	std::vector<std::string> folders;
	for (auto& folder : uniq_folders)
		folders.push_back(folder);
	std::sort(folders.begin(), folders.end(), compareLen);

	std::vector<std::string> intermediate_folders;

	// rmdir (only works if folders are empty!) out all uniq dirs...
	std::string fsroot(ROOT_PATH);
	for (auto& folder : folders)
	{
		auto parent = dir_name(folder);
		while (parent != "")
		{
			std::cout << "processing... " << parent << "\n";
			if ((uniq_folders.find(parent) == uniq_folders.end()) && (parent.length() > fsroot.length()))
			{
				std::cout << "adding " << parent << "\n";

				// folder not already seen, track it
				uniq_folders.insert(parent);
				intermediate_folders.push_back(parent);
			}
			parent = dir_name(parent);
		}
	}

	// have to re-add these outside of the loop because we can't
	// modify the vector while iterating through it
	for (auto& folder : intermediate_folders)
		folders.push_back(folder);

	//re-sort it
	std::sort(folders.begin(), folders.end(), compareLen);

	for (auto& folder : folders)
		rmdir(folder.c_str());

	printf("Removing manifest...\n");

	ManifestFile.close();

	std::remove(ManifestPath.c_str());
	std::remove((std::string(pkg_path) + this->pkg_name + "/info.json").c_str());

	rmdir((std::string(pkg_path) + this->pkg_name).c_str());

	// package removed, clean up empty directories
	// TODO: potentially prompt user to remove some known config files for a given package
	// see: https://github.com/vgmoose/get/issues/1
	// remove_empty_dirs(ROOT_PATH, 0);

	printf("Homebrew removed\n");

	return true;
}

void Package::updateStatus(const char* pkg_path)
{
	// check if the manifest for this package exists
	std::string ManifestPathInternal = "manifest.install";
	std::string ManifestPath = pkg_path + this->pkg_name + "/" + ManifestPathInternal;

	struct stat sbuff;
	if (stat(ManifestPath.c_str(), &sbuff) == 0)
	{
		// manifest exists, we are at least installed
		this->status = INSTALLED;
	}

	// TODO: check for info.json, parse version out of it
	// and compare against the package's to know whether
	// it's an update or not
	std::string jsonPathInternal = "info.json";
	std::string jsonPath = pkg_path + this->pkg_name + "/" + jsonPathInternal;

	if (INSTALLED && stat(jsonPath.c_str(), &sbuff) == 0)
	{
		// pull out the version number and check if it's
		// different than the one on the repo
		std::ifstream ifs(jsonPath.c_str());
		rapidjson::IStreamWrapper isw(ifs);

		if (!ifs.good())
		{
			printf("--> Could not locate %s", jsonPath.c_str());
			this->status = UPDATE; // issue opening info.json, assume update
			return;
		}

		rapidjson::Document doc;
		rapidjson::ParseResult ok = doc.ParseStream(isw);
		std::string version;

		if (ok && doc.HasMember("version"))
		{
			const rapidjson::Value& info_doc = doc["version"];
			version = info_doc.GetString();
		}
		else
			version = "0.0.0";

		if (version != this->version)
			this->status = UPDATE;

		// we're eithe ran update or an install at this point
		return;
	}
	else if (this->status == INSTALLED)
	{
		this->status = UPDATE; // manifest, but no info, always update
		return;
	}

	// if we're down here, and it's not a local package
	// already, it's probably a get package (package was
	// available, but the manifest wasn't installed)
	if (this->status != LOCAL)
		this->status = GET;

	// check for any homebrew that may have been previously installed
	// TODO: see https://github.com/vgmoose/hb-appstore/issues/20
	this->status = this->isPreviouslyInstalled();
}

int Package::isPreviouslyInstalled()
{
#if defined(__WIIU__)
	// we're on a Wii U, so let's check for any HBL meta.xml files that match this package's name,
	// and if it exists check the version based on that
	TiXmlDocument xmlDoc((std::string(ROOT_PATH) + "wiiu/apps/" + this->pkg_name + "/meta.xml").c_str());
	bool xmlExists = xmlDoc.LoadFile();

	if (xmlExists)
	{
		TiXmlElement* appNode = xmlDoc.FirstChildElement("app");
		if (appNode)
		{
			TiXmlElement* node = appNode->FirstChildElement("version");
			if (node && node->FirstChild() && node->FirstChild()->Value())
			{
				// version exists, we should compare the value to the one on the server (this package)
				if (this->version != node->FirstChild()->Value())
					return UPDATE;
				else
					return LOCAL;
			}
		}
	}
#endif

	// since we are appstore and know that what version we're supposed to be, mark us local or updated if needed
	// TODO: make version check here dynamic, and also support other NROs or hint files
	// notice: this means that even if appstore isn't installed but is running, it will show as an update
	if (this->pkg_name == "appstore")
	{
		// it's app store, but wasn't detected as installed
		if (this->version == "2.0")
			return LOCAL;
		else
			return UPDATE;
	}

	return this->status;
}

const char* Package::statusString()
{
	switch (this->status)
	{
	case LOCAL:
		return "LOCAL";
	case INSTALLED:
		return "INSTALLED";
	case UPDATE:
		return "UPDATE";
	case GET:
		return "GET";
	}
	return "UNKNOWN";
}
