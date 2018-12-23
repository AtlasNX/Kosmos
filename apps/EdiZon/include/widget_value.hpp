#pragma once

#include "widget.hpp"

#include <switch.h>

class WidgetValue : public Widget {
public:
  WidgetValue(ScriptParser *saveParser, std::string readEquation, std::string writeEquation, s64 minValue, s64 maxValue, u64 stepSize);
  ~WidgetValue();

  void draw(Gui *gui, u16 x, u16 y);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  s64 m_currValue;
  std::string m_readEquation, m_writeEquation;
  s64 m_minValue, m_maxValue;
  u64 m_stepSize;
};
