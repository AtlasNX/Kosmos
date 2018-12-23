#pragma once

#include <switch.h>

#include <functional>
#include <vector>

#include "gui.hpp"

class Button {
public:
  static inline std::vector<Button*> g_buttons;

  Button(u16 x, u16 y, u16 w, u16 h, std::function<void(Gui*, u16, u16, bool*)> drawAction, std::function<void(u32, bool*)> inputAction, std::vector<s16> adjacentButton, bool activatable);

  void draw(Gui *gui);
  bool onInput(u32 kdown);
  void onTouch(touchPosition &touch);

  inline bool isActivated() {
    return m_isActivated;
  }

  inline bool isSelected() {
    return m_isSelected;
  }

  static inline void select(s16 buttonIndex) {
    if (buttonIndex < 0) return;

    for(Button *btn : Button::g_buttons) {
      btn->m_isSelected = false;
      btn->m_isActivated = false;
    }


    Button::g_buttons[buttonIndex]->m_isSelected = true;
  }

private:

  u16 m_x, m_y, m_w, m_h;

  std::function<void(Gui*, u16, u16, bool*)> m_drawAction;
  std::function<void(u32, bool*)> m_inputAction;
  std::vector<s16> m_adjacentButton;

  bool m_isActivated;
  bool m_isSelected;
  bool m_activatable;
};
