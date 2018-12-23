#pragma once

#include <switch.h>
#include <string>

class Gui;

class Snackbar {
public:
  Snackbar(std::string text);
  ~Snackbar();

  void show();

  void update();
  void draw(Gui *gui);

  bool isDead();

private:
  std::string m_text;
  u16 m_displayTime;
  bool m_isDead;
};
