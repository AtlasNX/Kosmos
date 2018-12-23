#include "widget_list.hpp"

#include "list_selector.hpp"

WidgetList::WidgetList(ScriptParser *saveParser, std::vector<std::string> listItemNames, std::vector<s32> listItemValues) :
 Widget(saveParser), m_listItemNames(listItemNames), m_intListItemValues(listItemValues) {
  m_widgetDataType = INT;
}

WidgetList::WidgetList(ScriptParser *saveParser, std::vector<std::string> listItemNames, std::vector<std::string> listItemValues) :
 Widget(saveParser), m_listItemNames(listItemNames), m_strListItemValues(listItemValues) {
  m_widgetDataType = STRING;
}

WidgetList::~WidgetList() {

}

void WidgetList::draw(Gui *gui, u16 x, u16 y) {
  std::stringstream ss;
  if (m_widgetDataType == INT) {
    if (std::find(m_intListItemValues.begin(), m_intListItemValues.end(), Widget::getIntegerValue()) != m_intListItemValues.end()) {
      ptrdiff_t pos = find(m_intListItemValues.begin(), m_intListItemValues.end(), Widget::getIntegerValue()) - m_intListItemValues.begin();
      ss << m_listItemNames[pos];
    } else {
      ss << "Unknown value: " << Widget::getIntegerValue();
    }
  } else if (m_widgetDataType == STRING) {
    if (std::find(m_strListItemValues.begin(), m_strListItemValues.end(), Widget::getStringValue()) != m_strListItemValues.end()) {
      ptrdiff_t pos = find(m_strListItemValues.begin(), m_strListItemValues.end(), Widget::getStringValue()) - m_strListItemValues.begin();
      ss << m_listItemNames[pos];
    } else {
      ss << "Unknown value: " << Widget::getStringValue();
    }
  }

  gui->drawTextAligned(font20, x + WIDGET_WIDTH - 140, y + (WIDGET_HEIGHT / 2.0F), currTheme.selectedColor, ss.str().c_str(), ALIGNED_RIGHT);
}

void WidgetList::onInput(u32 kdown) {
  if (kdown & KEY_A && Gui::g_currListSelector == nullptr) {
    (new ListSelector("Choose item", "\uE0E0 - Select      \uE0E1 - Back", m_listItemNames))->setInputAction([&](u32 k, u16 selectedItem){
      if(k & KEY_A) {
        if (m_widgetDataType == INT)
          Widget::setIntegerValue(m_intListItemValues[selectedItem]);
        else if (m_widgetDataType == STRING)
          Widget::setStringValue(m_strListItemValues[selectedItem]);
        Gui::g_currListSelector->hide();
      }
    })->show();
  }
}

void WidgetList::onTouch(touchPosition &touch) {
  (new ListSelector("Choose item", "\uE0E0 - Select      \uE0E1 - Back", m_listItemNames))->setInputAction([&](u32 k, u16 selectedItem){
    if(k & KEY_A) {
      if (m_widgetDataType == INT)
        Widget::setIntegerValue(m_intListItemValues[selectedItem]);
      else if (m_widgetDataType == STRING)
        Widget::setStringValue(m_strListItemValues[selectedItem]);
      Gui::g_currListSelector->hide();
    }
  })->show();
}
