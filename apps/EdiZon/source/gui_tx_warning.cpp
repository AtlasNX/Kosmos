#include "gui_tx_warning.hpp"

GuiTXWarning::GuiTXWarning() : Gui() {
  hideWarning = false;
}

GuiTXWarning::~GuiTXWarning() {

}

void GuiTXWarning::update() {
  Gui::update();
}

void GuiTXWarning::draw() {
  Gui::beginDraw();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0xC5, 0x39, 0x29, 0xFF));
  Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, COLOR_WHITE, "\uE150", ALIGNED_CENTER);

  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, COLOR_WHITE, "EdiZon detected that you're running the SX OS custom firmware. Please note that this\nmay cause unexpected failures, corruption of save data or backups, the Editor failing\nto load save files or configs and many other issues. For your own safety and the\n safety of your Nintendo Switch, please use Atmosphère instead.", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 150 , COLOR_WHITE, "If you want to proceed anyways, press \uE0E0. Otherwise press \uE0EF to exit.", ALIGNED_CENTER);

  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 250, COLOR_WHITE, "\uE070  Don't show this warning anymore", ALIGNED_CENTER);

  if (!hideWarning)
    Gui::drawRectangle(Gui::g_framebuffer_width / 2 - 228, Gui::g_framebuffer_height / 2 + 258, 14, 16, Gui::makeColor(0xC5, 0x39, 0x29, 0xFF));

  Gui::endDraw();
}

void GuiTXWarning::onInput(u32 kdown) {
  if (kdown & KEY_A) {
    Gui::g_nextGui = GUI_MAIN;
    if (hideWarning) {
      FILE *fp = fopen("/EdiZon/.hide_sxos", "ab+");
      fclose(fp);
    }
  }
}

void GuiTXWarning::onTouch(touchPosition &touch) {
  if (touch.px > 400 && touch.px < 900 && touch.py > 600 && touch.py < 660)
    hideWarning = !hideWarning;
}

void GuiTXWarning::onGesture(touchPosition &startPosition, touchPosition &endPosition) {

}
