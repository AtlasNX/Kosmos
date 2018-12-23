#ifndef REPO_H
#define REPO_H
#include "Package.hpp"
#include <iostream>
#include <vector>

class Repo
{
public:
	Repo();
	Repo(const char* name, const char* url);
	std::string toJson();
	std::string toString();
	void loadPackages(std::vector<Package*>* package);

	std::string name;
	std::string url;
	bool enabled;
	bool loaded = true; // whether this server could be reached or not
};

std::string generateRepoJson(int count, ...);
#endif
