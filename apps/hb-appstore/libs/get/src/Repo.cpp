#include "Repo.hpp"
#include "Utils.hpp"
#include "constants.h"
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include <regex>
#include <sstream>
#include <stdarg.h> /* va_list, va_start, va_arg, va_end */

using namespace rapidjson;

Repo::Repo()
{
}

Repo::Repo(const char* name, const char* url)
{
	// create a repo from the passed parameters
	this->name = name;
	this->url = url;
	this->enabled = true;
}

std::string Repo::toJson()
{
	std::stringstream resp;
	resp << "\t\t{\n\t\t\t\"name\": \"" << this->name << "\",\n\t\t\t\"url\": \"" << this->url << "\",\n\t\t\t\"enabled\": " << (this->enabled ? "true" : "false") << "\n\t\t}\n";
	return resp.str();
}

std::string generateRepoJson(int count, ...)
{
	va_list ap;

	std::stringstream response;
	response << "{\n\t\"repos\": [\n";

	va_start(ap, count);

	for (int x = 0; x < count; x++)
		response << (va_arg(ap, Repo*))->toJson();

	va_end(ap);
	response << "\t]\n}\n";

	return response.str();
}

std::string Repo::toString()
{
	return "[" + this->name + "] <" + this->url + "> - " + ((this->enabled) ? "enabled" : "disabled");
}

void Repo::loadPackages(std::vector<Package*>* packages)
{
	std::string directoryUrl = this->url + "/repo.json";

	// fetch current repository json
	std::string response;
	bool success = downloadFileToMemory(directoryUrl, &response);

	std::string* response_copy = new std::string(response);

	if (!success)
	{
		printf("--> Could not update repository metadata for \"%s\" repo!\n", this->name.c_str());
		this->loaded = false;
		return;
	}

	// extract out packages, append to package list
	Document doc;
	ParseResult ok = doc.Parse(response_copy->c_str());

	if (!ok || !doc.IsObject() || !doc.HasMember("packages"))
	{
		printf("--> Invalid format in downloaded repo.json for %s\n", this->url.c_str());
		this->loaded = false;
		return;
	}

	const Value& packages_doc = doc["packages"];

	// for every repo
	for (Value::ConstValueIterator it = packages_doc.Begin(); it != packages_doc.End(); it++)
	{
		Package* package = new Package(GET);

		// TODO: use arrays and loops for parsing this info, and also check the type first

		// mostly essential attributes
		package->pkg_name = (*it)["name"].GetString();
		if ((*it).HasMember("title"))
			package->title = (*it)["title"].GetString();
		else
			package->title = package->pkg_name;
		if ((*it).HasMember("author"))
			package->author = (*it)["author"].GetString();
		if ((*it).HasMember("description"))
			package->short_desc = (*it)["description"].GetString();
		if ((*it).HasMember("details"))
			package->long_desc = std::regex_replace((*it)["details"].GetString(), std::regex("\\\\n"), "\n");
		if ((*it).HasMember("version"))
			package->version = (*it)["version"].GetString();

		// more information and details
		if ((*it).HasMember("license"))
			package->license = (*it)["license"].GetString();
		if ((*it).HasMember("changelog"))
			package->changelog = std::regex_replace((*it)["changelog"].GetString(), std::regex("\\\\n"), "\n");
		if ((*it).HasMember("url"))
			package->url = (*it)["url"].GetString();
		if ((*it).HasMember("updated"))
		{
			package->updated = (*it)["updated"].GetString();
			struct tm tm;
			time_t ts;

			auto res = strptime(package->updated.c_str(), "%d/%m/%Y", &tm);
			if (res)
			{
				ts = mktime(&tm);
				package->updated_timestamp = (int)ts;
			}
		}

		// even more details
		if ((*it).HasMember("app_dls"))
			package->downloads += (*it)["app_dls"].GetInt();
		if ((*it).HasMember("web_dls"))
			package->downloads += (*it)["web_dls"].GetInt();
		if ((*it).HasMember("extracted"))
			package->extracted_size += (*it)["extracted"].GetInt();
		if ((*it).HasMember("filesize"))
			package->download_size += (*it)["filesize"].GetInt();

		if ((*it).HasMember("category"))
			package->category = (*it)["category"].GetString();
		if ((*it).HasMember("binary"))
			package->binary = (*it)["binary"].GetString();
		package->repoUrl = &this->url;

		// save the response string to cleanup later
		package->contents = response_copy;

		packages->push_back(package);
	}
}
