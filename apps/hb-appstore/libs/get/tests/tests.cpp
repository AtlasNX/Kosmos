#include "../src/Get.hpp"
#include "../src/Utils.hpp"
#include <cstdio>
#include <sys/stat.h>
#include <vector>

#define URL "https://vgmoose.github.io/get/tests/server/"
//#define URL "http://localhost:8000/server/"

using namespace std;

int count(Get* get, int state)
{
	int count = 0;
	for (int x = 0; x < get->packages.size(); x++)
		if (get->packages[x]->status == state)
			count++;
	return count;
}

bool install(Get* get, const char* name)
{
	for (int x = 0; x < get->packages.size(); x++)
		if (get->packages[x]->pkg_name == name)
			return get->install(get->packages[x]);

	return false;
}

bool remove(Get* get, const char* name)
{
	for (int x = 0; x < get->packages.size(); x++)
		if (get->packages[x]->pkg_name == name)
			return get->remove(get->packages[x]);

	return false;
}

bool exists(const char* path)
{
	struct stat sbuff;
	return (stat(path, &sbuff) == 0);
}

int main()
{
	init_networking();

	// create a Get object
	Get* get = new Get("./tests/.get/", URL "a");
	vector<Package*> packages = get->packages;

	//    // make sure that there are no packages available
	//    if (packages.size() != 0)
	//    {
	//        cout << "There shouldn't be any packages available, found " << packages.size() << endl;
	//        return 1;
	//    }
	//
	//    // enable the first repo
	//    get->toggleRepo(get->repos[0]);

	// make sure there are 3 installed packages
	if (packages.size() != 3)
	{
		cout << "There should be 3 packages enabled, found " << packages.size() << endl;
		return 2;
	}

	// install two of the packages
	get->install(get->packages[0]);
	get->install(get->packages[2]);

	// verify that some files got created
	// TODO: verify some

	// there should be 2 installed packages, 1 get package, 0 updates
	if (count(get, INSTALLED) != 2 || count(get, GET) != 1 || count(get, UPDATE) != 0)
	{
		cout << "There should be two packages installed, found " << count(get, INSTALLED) << endl;
		return 3;
	}

	// enable the second repo
	get->toggleRepo(get->repos[1]);

	// make sure that there are 5 total packages
	//    if (packages.size() != 5)
	//    {
	//        cout << "There should be 5 packages enabled, found " << packages.size() << endl;
	//        return 2;
	//    }

	// there should be 1 installed package, 3 get packages, and 1 update (TODO: add update between multiple repos)
	if (count(get, INSTALLED) != 2 /*2*/ || count(get, GET) != 3 || count(get, UPDATE) != 0 /*1*/)
	{
		cout << "There should be 1 package installed and 1 update, found " << count(get, INSTALLED) << endl;
		return 3;
	}

	// install one of the new packages, upgrade the other
	install(get, "three");
	install(get, "four");

	// there should be 3 installed packages, 2 get packages, and 0 updates
	if (count(get, INSTALLED) != 3 || count(get, GET) != 2 || count(get, UPDATE) != 0)
	{
		cout << "There should be 3 packages installed, found " << count(get, INSTALLED) << endl;
		return 3;
	}

	// try to do a search for a known string
	vector<Package*> results = get->search("t");

	// must be size two ("two", and "three")
	if (results.size() != 2)
	{
		cout << "Searching for \"t\" returned " << results.size() << " results instead of 2" << endl;
		return 4;
	}

	// make sure that empty folders are cleaned up afterwards... when removing a package
	remove(get, "four");
	if (exists("./sdroot/folder1") || exists("./sdroot/folder2") || exists("./sdroot/folder3"))
	{
		cout << "Tried to remove \"four\", but directory remained behind (should be empty)." << endl;
		return 5;
	}

	// make sure it goes down (total installed/updated package count)
	int sum = count(get, INSTALLED) + count(get, UPDATE);
	if (sum != 2)
	{
		cout << "There should only be 2 packages installed after removing one" << endl;
		return 6;
	}

	// tests passed!
	return 0;
}
