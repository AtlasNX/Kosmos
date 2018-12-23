#include "ImageCache.hpp"
#include "../libs/get/src/libs/rapidjson/include/rapidjson/document.h"
#include "../libs/get/src/libs/rapidjson/include/rapidjson/istreamwrapper.h"
#include "../libs/get/src/libs/rapidjson/include/rapidjson/writer.h"
#include "../libs/get/src/libs/rapidjson/include/rapidjson/stringbuffer.h"

#include <fstream>

std::unordered_map<std::string, SDL_Texture*> ImageCache::cache;
std::string ImageCache::cache_path;

ImageCache::ImageCache(const char* tmp_path)
{
	// create cache directory in the tmp folder
	ImageCache::cache_path = std::string(tmp_path) + "cache/";
	mkdir(cache_path.c_str(), 0700);

	// try to load version mapping list from disk
	rapidjson::Document doc;
	std::ifstream ifs(ImageCache::cache_path + "versions.json");
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::ParseResult ok = doc.ParseStream(isw);

	if (ok && doc.IsObject())
	{
		// go through this json doc and load strings into our map
		const rapidjson::Value& version_doc = doc;
		for(rapidjson::Value::ConstMemberIterator it=version_doc.MemberBegin(); it != version_doc.MemberEnd(); it++)
		{
			std::string pkg_name = it->name.GetString();
			std::string version = it->value.GetString();

			this->version_cache[pkg_name] = version;
		}
	}

	ifs.close();
}

void ImageCache::writeVersionCache()
{
	// write out the version cache to a file to load later
	
	rapidjson::Document doc;
	doc.SetObject();
	auto& allocator = doc.GetAllocator();
	
	for (auto& pair : this->version_cache) {
		rapidjson::Value key(pair.first.c_str(), allocator);
		rapidjson::Value value(pair.second.c_str(), allocator);
		doc.AddMember(key, value, allocator);
	}
	
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	
	std::ofstream ofs(ImageCache::cache_path + "versions.json");
	ofs << buffer.GetString();
	ofs.close();
}
