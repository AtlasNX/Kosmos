#ifndef GET_H
#define GET_H
#include "Repo.hpp"
#include "constants.h"
#include <vector>

class Get
{
public:
	// constructor takes path to the .get directory, and a fallback default repo url
	Get(const char* config_dir, const char* defaultRepo);

	int install(Package* pkg_name); // download the given package name and manifest data
	int remove(Package* pkg_name);  // delete and remove all files for the given package name
	int toggleRepo(Repo* repo);		// enable/disable the specified repo (and write changes)

	std::vector<Package*> search(std::string query); // return a list of packages matching query

	// the remote repos and packages
	std::vector<Repo*> repos;
	std::vector<Package*> packages;

	// map of word -> list of packages whose info matches that word
	// TODO: this
	// std::map<std::string, std::vector<Package*>>;

	// TODO: add queue functionality
	//	  void enqueue(int count, ...)	// add a number of packages to the download queue
	//	  void downloadAll()			// download all of the queued packages

	// config paths (TODO: replace with a Config class)
	const char* repos_path;
	const char* pkg_path;
	const char* tmp_path;

private:
	void loadRepos();
	void update();
	int validateRepos();

	const char* defaultRepo;
};
#endif
