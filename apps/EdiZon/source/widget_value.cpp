#include "widget_value.hpp"

#include <math.h>

#define ACCELERATION_DELAY 50

WidgetValue::WidgetValue(ScriptParser *saveParser, std::string readEquation, std::string writeEquation, s64 minValue, s64 maxValue, u64 stepSize) :
 Widget(saveParser), m_readEquation(readEquation), m_writeEquation(writeEquation), m_minValue(minValue), m_maxValue(maxValue), m_stepSize(stepSize) {
  m_widgetDataType = INT;

  m_currValue = 0;

}

WidgetValue::~WidgetValue() {

}

void WidgetValue::draw(Gui *gui, u16 x, u16 y) {
  std::stringstream ss;
  ss << m_currValue;

  if (m_currValue == 0)
    m_currValue = Widget::m_saveParser->evaluateEquation(m_readEquation, Widget::getIntegerValue());

  gui->drawTextAligned(font20, x + WIDGET_WIDTH - 140, y + (WIDGET_HEIGHT / 2.0F), currTheme.selectedColor, ss.str().c_str(), ALIGNED_RIGHT);
}

void WidgetValue::onInput(u32 kdown) {
  u64 incrementValue = m_stepSize * g_stepSizeMultiplier;

  if (kdown & KEY_LEFT) {
    if (static_cast<s64>(m_currValue - incrementValue) > m_minValue)
        Widget::setIntegerValue(Widget::m_saveParser->evaluateEquation(m_writeEquation, m_currValue) - incrementValue);
    else if(m_currValue == m_minValue)
      Widget::setIntegerValue(Widget::m_saveParser->evaluateEquation(m_writeEquation, m_maxValue));
    else
      Widget::setIntegerValue(Widget::m_saveParser->evaluateEquation(m_writeEquation, m_minValue));
  }

  if (kdown & KEY_RIGHT) {
    if (static_cast<s64>(m_currValue + incrementValue) < m_maxValue)
      Widget::setIntegerValue(Widget::m_saveParser->evaluateEquation(m_writeEquation, m_currValue) + incrementValue);
    else if(m_currValue == m_maxValue)
      Widget::setIntegerValue(Widget::m_saveParser->evaluateEquation(m_writeEquation, m_minValue));
    else
      Widget::setIntegerValue(Widget::m_saveParser->evaluateEquation(m_writeEquation, m_maxValue));
  }

  m_currValue = Widget::m_saveParser->evaluateEquation(m_readEquation, Widget::getIntegerValue());
}

void WidgetValue::onTouch(touchPosition &touch) {

}
