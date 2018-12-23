#pragma once

#include "widget.hpp"

#include <switch.h>

class WidgetSwitch : public Widget {
public:
  WidgetSwitch(ScriptParser *saveParser, s32 onValue, s32 offValue);
  WidgetSwitch(ScriptParser *saveParser, std::string onValue, std::string offValue);

  ~WidgetSwitch();

  void draw(Gui *gui, u16 x, u16 y);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  s32 m_intOnValue, m_intOffValue;
  std::string m_strOnValue, m_strOffValue;

};
