#pragma once

#include <switch.h>

#include <string>
#include <utility>
#include <vector>
#include <map>
#include <set>

#include "gui.hpp"
#include "script_parser.hpp"

#define WIDGET_WIDTH      900
#define WIDGET_HEIGHT     65
#define WIDGET_SEPARATOR  10
#define WIDGETS_PER_PAGE  6.0F
#define X_OFFSET          150

typedef enum WidgetDataType {
  INT,
  STRING
} WidgetDataType;

class Widget;
typedef struct { std::string title; Widget *widget; } WidgetItem;
typedef std::map<std::string, std::vector<WidgetItem>> WidgetItems;

typedef enum { CATEGORIES, WIDGETS } WidgetRow;

class Widget {
public:
  Widget(ScriptParser *saveParser);
  virtual ~Widget();

  static inline u16 g_selectedWidgetIndex = 0;
  static inline std::string g_selectedCategory;
  static inline std::vector<std::string> g_categories;
  static inline WidgetRow g_selectedRow = CATEGORIES;
  static inline u16 g_widgetPage;
  static inline std::map<std::string, u16> g_widgetPageCnt;
  static inline u8 g_categoryYOffset = 0;

  static void drawWidgets(Gui *gui, WidgetItems &widgets, u16 y, u16 start, u16 end);
  static void handleInput(u32 kdown, WidgetItems &widgets);

  virtual void draw(Gui *gui, u16 x, u16 y) = 0;
  virtual void onInput(u32 kdown) = 0;
  virtual void onTouch(touchPosition &touch) = 0;

  s64 getIntegerValue();
  std::string getStringValue();
  void setIntegerValue(s64 value);
  void setStringValue(std::string value);

  void setLuaArgs(std::vector<s32> intArgs, std::vector<std::string> strArgs);

  static inline u16 g_stepSizeMultiplier = 1;

protected:
  ScriptParser *m_saveParser;
  WidgetDataType m_widgetDataType;
  std::vector<s32> m_intArgs;
  std::vector<std::string> m_strArgs;
};
