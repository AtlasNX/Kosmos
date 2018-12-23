#include "widget_switch.hpp"

WidgetSwitch::WidgetSwitch(ScriptParser *saveParser, s32 onValue, s32 offValue) :
 Widget(saveParser), m_intOnValue(onValue), m_intOffValue(offValue) {
  m_widgetDataType = INT;
}

WidgetSwitch::WidgetSwitch(ScriptParser *saveParser, std::string onValue, std::string offValue) :
 Widget(saveParser), m_strOnValue(onValue), m_strOffValue(offValue){
  m_widgetDataType = STRING;
}


WidgetSwitch::~WidgetSwitch() {

}

void WidgetSwitch::draw(Gui *gui, u16 x, u16 y) {
  if (m_widgetDataType == INT)
    gui->drawTextAligned(font20, x + WIDGET_WIDTH - 140, y + (WIDGET_HEIGHT / 2.0F), Widget::getIntegerValue() == m_intOnValue ? currTheme.selectedColor : currTheme.separatorColor, Widget::getIntegerValue() == m_intOnValue ? "ON" : "OFF", ALIGNED_RIGHT);
  else if (m_widgetDataType == STRING)
    gui->drawTextAligned(font20, x + WIDGET_WIDTH - 140, y + (WIDGET_HEIGHT / 2.0F), Widget::getStringValue() == m_strOnValue ? currTheme.selectedColor : currTheme.separatorColor, Widget::getStringValue() == m_strOnValue ? "ON" : "OFF", ALIGNED_RIGHT);
}

void WidgetSwitch::onInput(u32 kdown) {
  if (kdown & KEY_A) {
    if (m_widgetDataType == INT) {
      if (Widget::getIntegerValue() == m_intOnValue)
        Widget::setIntegerValue(m_intOffValue);
      else
        Widget::setIntegerValue(m_intOnValue);
    } else if (m_widgetDataType == STRING) {
      if (Widget::getStringValue() == m_strOnValue)
        Widget::setStringValue(m_strOffValue);
      else
        Widget::setStringValue(m_strOnValue);
    }
  }
}

void WidgetSwitch::onTouch(touchPosition &touch) {
  if (m_widgetDataType == INT) {
    if (Widget::getIntegerValue() == m_intOnValue)
      Widget::setIntegerValue(m_intOffValue);
    else
      Widget::setIntegerValue(m_intOnValue);
  } else if (m_widgetDataType == STRING) {
    if (Widget::getStringValue() == m_strOnValue)
      Widget::setStringValue(m_strOffValue);
    else
      Widget::setStringValue(m_strOnValue);
  }
}
