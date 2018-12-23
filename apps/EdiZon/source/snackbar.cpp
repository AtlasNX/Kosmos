#include "snackbar.hpp"

#include "gui.hpp"

#include <cmath>

#define SNACKBAR_HEIGHT 100
#define DISPLAY_TIME    3000

u16 yOffset = SNACKBAR_HEIGHT;
u16 yOffsetNext = 0;

Snackbar::Snackbar(std::string text) : m_text(text) {
  yOffset = SNACKBAR_HEIGHT;
  yOffsetNext = 0;
  m_isDead = false;
}

Snackbar::~Snackbar() {

}

void Snackbar::show() {
  m_displayTime = DISPLAY_TIME;

  if (Gui::g_currSnackbar != nullptr)
    delete Gui::g_currSnackbar;

  Gui::g_currSnackbar = this;
}

void Snackbar::update() {
    float deltaOffset = yOffsetNext - yOffset;
    float scrollSpeed = deltaOffset / 1024.0F;

    if (yOffset != yOffsetNext) {
      if (yOffsetNext > yOffset)
        yOffset += ceil((abs(deltaOffset) > abs(scrollSpeed)) ? scrollSpeed : deltaOffset);
      else
        yOffset += floor((abs(deltaOffset) > abs(scrollSpeed)) ? scrollSpeed : deltaOffset);
    }

    if (m_displayTime > 0) m_displayTime--;
    else yOffsetNext = SNACKBAR_HEIGHT;

    if (yOffset == SNACKBAR_HEIGHT) m_isDead = true;
}

void Snackbar::draw(Gui *gui) {
  gui->drawRectangle(0, (Gui::g_framebuffer_height - SNACKBAR_HEIGHT) + yOffset, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.textColor);
  gui->drawText(font20, 50, (Gui::g_framebuffer_height - SNACKBAR_HEIGHT) + 35 + yOffset, currTheme.backgroundColor, m_text.c_str());
}

bool Snackbar::isDead() {
  return m_isDead;
}
