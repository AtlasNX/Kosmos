#include "widget.hpp"

#include "gui_editor.hpp"

#include <iostream>

Widget::Widget(ScriptParser *saveParser) : m_saveParser(saveParser) {
  Widget::g_stepSizeMultiplier = 1;
}

Widget::~Widget() {
  Widget::g_stepSizeMultiplier = 1;
}

void Widget::drawWidgets(Gui *gui, WidgetItems &widgets, u16 y, u16 start, u16 end) {
  if (Widget::g_categories.empty() || widgets.empty()) return;
  if (widgets.find(Widget::g_selectedCategory) == widgets.end()) return;

  ptrdiff_t categoryIndex = std::find(Widget::g_categories.begin(), Widget::g_categories.end(), Widget::g_selectedCategory) - Widget::g_categories.begin() - g_categoryYOffset;
  if (Widget::g_selectedRow == CATEGORIES) {
    gui->drawRectangled(25, y + 4 + 60 * categoryIndex, 310, 55, currTheme.highlightColor);
    gui->drawRectangle(30, y + 9 + 60 * categoryIndex, 300, 45, currTheme.selectedButtonColor);
    gui->drawShadow(25, y + 4 + 60 * categoryIndex, 310, 55);
  }

  gui->drawRectangle(37, y + 13 + 60 * categoryIndex, 4, 35, currTheme.selectedColor);

  for (u8 i = 0; i < Widget::g_categories.size(); i++) {
    gui->drawText(font20, 50, y + 15 + 60 * (i - g_categoryYOffset), Widget::g_categories[i] == Widget::g_selectedCategory ? currTheme.selectedColor : currTheme.textColor, Widget::g_categories[i].c_str());
  }

  std::vector<WidgetItem> &currWidgets = widgets[Widget::g_selectedCategory];

  if (currWidgets.size() <= 0) return;

  u16 widgetInset = (Gui::g_framebuffer_width - WIDGET_WIDTH) / 2.0F;

  for (;start < end; start++) {
    if (start > currWidgets.size() - 1) break;

    if (start == Widget::g_selectedWidgetIndex && Widget::g_selectedRow == WIDGETS) {
      gui->drawRectangled(widgetInset + X_OFFSET, y, WIDGET_WIDTH - 1, WIDGET_HEIGHT, currTheme.highlightColor);
      gui->drawRectangle(widgetInset + 5 + X_OFFSET, y + 5, WIDGET_WIDTH - 12, WIDGET_HEIGHT - 10, currTheme.selectedButtonColor);
      gui->drawShadow(widgetInset + X_OFFSET, y, WIDGET_WIDTH, WIDGET_HEIGHT);
    }

    gui->drawTextAligned(font20, widgetInset + 50 + X_OFFSET, y + (WIDGET_HEIGHT / 2.0F) - 13, currTheme.textColor, currWidgets[start].title.c_str(), ALIGNED_LEFT);
    gui->drawRectangle(widgetInset + 30 + X_OFFSET, y + WIDGET_HEIGHT + (WIDGET_SEPARATOR / 2) - 1, WIDGET_WIDTH - 60, 1, currTheme.separatorColor);
    currWidgets[start].widget->draw(gui, widgetInset + 50 + X_OFFSET, y - 13);

    y += WIDGET_HEIGHT + WIDGET_SEPARATOR;
  }
}

void Widget::handleInput(u32 kdown, WidgetItems &widgets) {
  std::vector<WidgetItem> &currWidgets = widgets[Widget::g_selectedCategory];

  if (currWidgets.size() > 0 && Widget::g_selectedRow == WIDGETS)
    currWidgets[Widget::g_selectedWidgetIndex].widget->onInput(kdown);

  if (Widget::g_selectedRow == CATEGORIES) {
    if (kdown & KEY_A || kdown & KEY_RIGHT) {
      Widget::g_selectedRow = WIDGETS;
      Widget::g_selectedWidgetIndex = Widget::g_widgetPage * WIDGETS_PER_PAGE;
    }
  }

  if (kdown & KEY_RSTICK) {
    if (g_stepSizeMultiplier == 10000) g_stepSizeMultiplier = 1;
    else g_stepSizeMultiplier *= 10;
  }
}

s64 Widget::getIntegerValue() {
  m_saveParser->setLuaArgs(m_intArgs, m_strArgs);

  return  m_saveParser->getValueFromSaveFile();
}

std::string Widget::getStringValue() {
  m_saveParser->setLuaArgs(m_intArgs, m_strArgs);
  return m_saveParser->getStringFromSaveFile();
}

void Widget::setIntegerValue(s64 value) {
  m_saveParser->setLuaArgs(m_intArgs, m_strArgs);
  m_saveParser->setValueInSaveFile(value);
}

void Widget::setStringValue(std::string value) {
  m_saveParser->setLuaArgs(m_intArgs, m_strArgs);
  m_saveParser->setStringInSaveFile(value);
}

void Widget::setLuaArgs(std::vector<s32> intArgs, std::vector<std::string> strArgs) {
  this->m_intArgs = intArgs;
  this->m_strArgs = strArgs;
}
