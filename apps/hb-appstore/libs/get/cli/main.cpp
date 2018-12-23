#include "../src/Get.hpp"
#include "../src/Utils.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

int main(int argc, char** args)
{
	init_networking();

	// create main Get object
	Get* get = new Get("./.get/", "http://switchbru.com/appstore");

	vector<Repo*> repos = get->repos;
	vector<Package*> packages = get->packages;

	bool removeMode = false;

	for (int x = 1; x < argc; x++)
	{
		std::string cur = args[x];

		if (cur == "-x" || cur == "-d" || cur == "-r" || cur == "--delete" || cur == "--remove")
		{
			removeMode = true;
		}
		else if (cur == "-s" || cur == "--search")
		{
			if (argc == x + 1)
			{
				printf("--> Please provide search query\n");
				break;
			}

			// get search lower_query
			std::string query = std::string(args[x + 1]);

			for (int y = x + 2; y < argc; y++)
				query += std::string(" ") + std::string(args[y]);

			printf("--> Searching for \"%s\" in all repos...\n", query.c_str());

			vector<Package*> results = get->search(query);

			for (int y = 0; y < results.size(); y++)
				printf("\t%s %s\n", results[y]->statusString(), results[y]->toString().c_str());

			break;
		}
		else if (cur == "-l" || cur == "--list")
		{
			// list available remote packages
			printf("--> Listing available remotes and packages\n");

			printf("%d repo%s loaded!\n", repos.size(), plural(repos.size()));
			for (int x = 0; x < repos.size(); x++)
				printf("\t%s\n", repos[x]->toString().c_str());

			printf("%d package%s available!\n", packages.size(), plural(packages.size()));
			for (int x = 0; x < packages.size(); x++)
				printf("\t%s %s\n", packages[x]->statusString(), packages[x]->toString().c_str());

			int count = 0;
			int updatecount = 0;
			for (int x = 0; x < packages.size(); x++)
			{
				if (packages[x]->status != GET)
					count++;
				if (packages[x]->status == UPDATE)
					updatecount++;
			}
			printf("%d package%s installed\n", count, plural(count));
			printf("%d update%s available\n", updatecount, plural(updatecount));
		}
		else // assume argument is a package
		{
			// try to find the package in a local repo
			// TODO: use a hash map to improve speed
			bool found = false;

			for (int y = 0; y < packages.size(); y++)
			{
				if (packages[y]->pkg_name == cur)
				{
					found = true;

					if (removeMode)
					{
						// remove flag was specified, delete this package
						get->remove(packages[y]);
						break;
					}

					get->install(packages[y]);
					break;
				}
			}

			if (!found)
				printf("--> No package named [%s] found in enabled repos!\n", cur.c_str());
		}
	}

	return 0;
}
