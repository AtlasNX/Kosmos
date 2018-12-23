#ifndef APPDETAILS_H_
#define APPDETAILS_H_

#include "TextElement.hpp"
#include "ImageElement.hpp"
#include "ProgressBar.hpp"
#include "ListElement.hpp"
#include "../libs/get/src/Package.hpp"
#include "../libs/get/src/Get.hpp"

class AppList;

class AppDetails : public Element
{
public:
	AppDetails(Package* package, AppList* appList);
	bool process(InputEvents* event);
	void render(Element* parent);
    
	bool operating = false;
	Package* package;
    Get* get;
    AppList* appList;
	ProgressBar* pbar = NULL;
	int highlighted = -1;
    
	// the callback method to update the currently displayed pop up (and variables it needs)
    static int updateCurrentlyDisplayedPopup(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    
    void proceed();
    void back();
    void moreByAuthor();
    void leaveFeedback();
    
    void preInstallHook();
    void postInstallHook();

};

class AppDetailsContent : public ListElement
{
    bool process(InputEvents* event);
    void render(Element* parent);
};

#endif
