#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <unordered_map>
#include <SDL2/SDL.h>

class ImageCache
{
public:

	ImageCache(const char* tmp_path);
	void writeVersionCache();

	// the path to the cache folder
	static std::string cache_path;

	// a map of all SDL surfaces that have been displayed
	static std::unordered_map<std::string, SDL_Texture*> cache;

	// a map of pkg_names to version strings to know when icons are outdated
	std::unordered_map<std::string, std::string> version_cache;
};
