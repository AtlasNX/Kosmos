#include <algorithm>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

#include "constants.h"

#include "Get.hpp"
#include "Utils.hpp"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace std;
using namespace rapidjson;

Get::Get(const char* config_dir, const char* defaultRepo)
{
	this->defaultRepo = defaultRepo;

	// the path for the get metadata folder
	string config_path = config_dir;

	string* repo_file = new string(config_path + "repos.json");
	this->repos_path = repo_file->c_str();
	string* package_dir = new string(config_path + "packages/");
	this->pkg_path = package_dir->c_str();
	string* tmp_dir = new string(config_path + "tmp/");
	this->tmp_path = tmp_dir->c_str();

	//	  printf("--> Using \"./sdroot\" as local download root directory\n");
	//	  mkdir("./sdroot", 0700);

	mkdir(config_path.c_str(), 0700);
	mkdir(package_dir->c_str(), 0700);
	mkdir(tmp_dir->c_str(), 0700);

	printf("--> Using \"%s\" as repo list\n", repo_file->c_str());

	// load repo info
	this->loadRepos();
	this->update();
}

int Get::install(Package* package)
{
	// found package in a remote server, fetch it
	bool located = package->downloadZip(this->tmp_path);

	if (!located)
	{
		// according to the repo list, the package zip file should've been here
		// but we got a 404 and couldn't find it
		printf("--> Error retrieving remote file for [%s] (check network or 404 error?)\n", package->pkg_name.c_str());
		return false;
	}

	// install the package, (extracts manifest, etc)
	package->install(this->pkg_path, this->tmp_path);

	printf("--> Downloaded [%s] to sdroot/\n", package->pkg_name.c_str());

	// update again post-install
	update();
	return true;
}

int Get::remove(Package* package)
{
	package->remove(this->pkg_path);
	printf("--> Uninstalled [%s] package\n", package->pkg_name.c_str());
	update();

	return true;
}

int Get::toggleRepo(Repo* repo)
{
	repo->enabled = !repo->enabled;
	update();
	return true;
}

/**
Load any repos from a config file into the repos vector.
**/
void Get::loadRepos()
{
	repos.clear();
	const char* config_path = repos_path;

	ifstream* ifs = new ifstream(config_path);

	if (!ifs->good() || ifs->peek() == std::ifstream::traits_type::eof())
	{
		printf("--> Could not load repos from %s, generating default repos.json\n", config_path);

		Repo* defaultRepo = new Repo("Default Repo", this->defaultRepo);

		Document d;
		d.Parse(generateRepoJson(1, defaultRepo).c_str());

		std::ofstream file(config_path);
		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		d.Accept(writer);
		file << buffer.GetString();

		ifs = new ifstream(config_path);

		if (!ifs->good())
		{
			printf("--> Could not generate a new repos.json\n");

			// manually create a repo, no file access
			Repo* repo = new Repo();
			repo->name = "Switchbru";
			repo->url = this->defaultRepo;
			repo->enabled = true;
			repos.push_back(repo);
			return;
		}
	}

	IStreamWrapper isw(*ifs);

	Document doc;
	ParseResult ok = doc.ParseStream(isw);

	if (!ok || !doc.HasMember("repos"))
	{
		printf("--> Invalid format in %s", config_path);
		return;
	}

	const Value& repos_doc = doc["repos"];

	// for every repo
	for (Value::ConstValueIterator it = repos_doc.Begin(); it != repos_doc.End(); it++)
	{
		Repo* repo = new Repo();
		repo->name = (*it)["name"].GetString();
		repo->url = (*it)["url"].GetString();
		repo->enabled = (*it)["enabled"].GetBool();
		repos.push_back(repo);
	}

	return;
}

void Get::update()
{
	// clear current packages
	packages.clear();

	// fetch recent package list from enabled repos
	for (int x = 0; x < repos.size(); x++)
	{
		if (repos[x]->enabled)
			repos[x]->loadPackages(&packages);
	}

	// check for any installed packages to update their status
	for (int x = 0; x < packages.size(); x++)
		packages[x]->updateStatus(this->pkg_path);
}

int Get::validateRepos()
{
	if (repos.size() == 0)
	{
		printf("There are no repos configured!\n");
		return ERR_NO_REPOS;
	}

	return 0;
}

std::vector<Package*> Get::search(std::string query)
{
	// TODO: replace with inverted index for speed
	// https://vgmoose.com/blog/implementing-a-static-blog-search-clientside-in-js-6629164446/

	std::vector<Package*> results = std::vector<Package*>();
	std::string lower_query = toLower(query);

	for (int x = 0; x < packages.size(); x++)
	{
		Package* cur = packages[x];
		if (cur != NULL && (toLower(cur->title).find(lower_query) != std::string::npos || toLower(cur->author).find(lower_query) != std::string::npos || toLower(cur->short_desc).find(lower_query) != std::string::npos || toLower(cur->long_desc).find(lower_query) != std::string::npos))
		{
			// matches, add to return vector, and continue
			results.push_back(cur);
			continue;
		}
	}

	return results;
}
