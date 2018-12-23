#include "TextElement.hpp"
#include "ImageElement.hpp"
#include "AppDetails.hpp"
#include "Sidebar.hpp"
#include "ListElement.hpp"
#include "Keyboard.hpp"
#include "../libs/get/src/Get.hpp"

#define TOTAL_SORTS 5   // alphabetical (with updates at top), downloads, last updated, size, shuffled
#define RECENT 0
#define POPULARITY 1
#define ALPHABETICAL 2
#define SIZE 3
#define RANDOM 4


class AppList : public ListElement
{
public:
	AppList(Get* get, Sidebar* sidebar);
	bool process(InputEvents* event);
	void render(Element* parent);
	void update();

	Get* get = NULL;
	Sidebar* sidebar = NULL;
    Keyboard* keyboard = NULL;

    void toggleKeyboard();
    void cycleSort();
    void reorient();
    
    const char* applySortOrder(std::vector<Package*>* packages);

	bool touchMode = true;

	// the total number of apps displayed in this list
	int totalCount = 0;

    // default number of items per row TODO: save this value as config
    int R = 3;

    int sortMode = RECENT;

    void launchSettings();

};
