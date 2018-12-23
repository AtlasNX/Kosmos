#include "TextElement.hpp"
#include "ImageElement.hpp"
#include "ListElement.hpp"

class AppList;

#define TOTAL_CATS 8
#pragma once

class Sidebar : public ListElement
{
public:
	Sidebar();

	std::string currentCatName();
	std::string currentCatValue();

	std::string searchQuery = "";

	AppList* appList = NULL;

	void render(Element* parent);
	bool process(InputEvents* event);

	// the currently selected category index
	int curCategory = 1;		// 1 is all apps

	// list of human-readable category titles and short names from the json
	const char* cat_names[TOTAL_CATS] = {"Search", "All Apps", "Games", "Emulators", "Tools", "Advanced", "Loaders", "Concepts"};
	const char* cat_value[TOTAL_CATS] = {"_search", "_all", "game", "emu", "tool", "advanced", "loader", "concept"};

};
