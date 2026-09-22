#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <EEPROM.h>
#include "TM1638_Custom.h"
#include "Buttons.h"

// Связываем переменные со скетчем
extern int minTemp1, maxTemp1, minTemp2, maxTemp2;
extern int turboTime;
extern int turboTemp;
extern bool SOUND;
extern bool out1_Enabled, out2_Enabled;

class MenuHandler {
  private:
    TM1638_Custom* _disp;
    byte _currentItem; // Номер пункта меню (от 0 до 8)

  public:
    MenuHandler(TM1638_Custom* disp) {
      _disp = disp;
      _currentItem = 0;
    }

    void reset() { _currentItem = 0; }

    bool handleMenu(ButtonEvent event) {
      if (event == CLICK_TURBO) {
        // Сохраняем настройки в EEPROM
        EEPROM.write(2, minTemp1);  EEPROM.write(3, maxTemp1);
        EEPROM.write(4, minTemp2);  EEPROM.write(5, maxTemp2);
        EEPROM.write(6, highByte(turboTime));
        EEPROM.write(7, lowByte(turboTime));
        EEPROM.write(8, turboTemp);
        EEPROM.write(9, SOUND ? 1 : 0);
        EEPROM.write(10, out1_Enabled ? 1 : 0);
        EEPROM.write(11, out2_Enabled ? 1 : 0);
        return true; // Выход из меню
      }

      // Навигация по 9 пунктам меню (от 0 до 8)
      if (event == CLICK_UP_1)   { if (_currentItem < 8) _currentItem++; else _currentItem = 0; }
      if (event == CLICK_DOWN_1) { if (_currentItem > 0) _currentItem--; else _currentItem = 8; }

      bool upPressed = (event == CLICK_UP_2 || event == HOLD_UP_2);
      bool downPressed = (event == CLICK_DOWN_2 || event == HOLD_DOWN_2);

      switch (_currentItem) {
        case 0: // t1_
          _disp->printString("t1_ ", true); _disp->printPower(minTemp1, false);
          if (upPressed && minTemp1 < maxTemp1) minTemp1 += 5;
          if (downPressed && minTemp1 > 10) minTemp1 -= 5;
          break;
        case 1: // t1~
          _disp->printString("t1~ ", true); _disp->printPower(maxTemp1, false);
          if (upPressed && maxTemp1 < 200) maxTemp1 += 5;
          if (downPressed && maxTemp1 > minTemp1) maxTemp1 -= 5;
          break;
        case 2: // t2_
          _disp->printString("t2_ ", true); _disp->printPower(minTemp2, false);
          if (upPressed && minTemp2 < maxTemp2) minTemp2 += 5;
          if (downPressed && minTemp2 > 10) minTemp2 -= 5;
          break;
        case 3: // t2~
          _disp->printString("t2~ ", true); _disp->printPower(maxTemp2, false);
          if (upPressed && maxTemp2 < 200) maxTemp2 += 5;
          if (downPressed && maxTemp2 > minTemp2) maxTemp2 -= 5;
          break;
        case 4: // Время ТУРБО (turboTime до 999 секунд с буквой S)
          _disp->printString("trbo", true); 
          
          // ИСПРАВЛЕНИЕ: Выделяем буфер с хорошим запасом (6 байт), чтобы он не затирал память!
          char secBuf[6]; 
          if (turboTime >= 100)      sprintf(secBuf, "%3ds", turboTime);
          else if (turboTime >= 10)  sprintf(secBuf, " %2ds", turboTime);
          else                       sprintf(secBuf, "  %1ds", turboTime);
          _disp->printString(secBuf, false);
          
          if (upPressed && turboTime < 999) turboTime += 1;
          if (downPressed && turboTime > 1) turboTime -= 1;
          break;
        case 5: // Температура ТУРБО
          _disp->printString("trbO", true); _disp->printPower(turboTemp, false);
          if (upPressed && turboTemp < 200) turboTemp += 5;
          if (downPressed && turboTemp > 50) turboTemp -= 5;
          break;
        case 6: // Звук
          _disp->printString("Snd ", true);
          if (SOUND) _disp->printString("On  ", false);
          else       _disp->printString("OFF ", false);
          if (event == CLICK_UP_2 || event == CLICK_DOWN_2) SOUND = !SOUND;
          break;
        case 7: // Выход 1
          _disp->printString("Out1", true);
          if (out1_Enabled) _disp->printString("On  ", false);
          else              _disp->printString("OFF ", false);
          if (event == CLICK_UP_2 || event == CLICK_DOWN_2) out1_Enabled = !out1_Enabled;
          break;
        case 8: // Выход 2
          _disp->printString("Out2", true);
          if (out2_Enabled) _disp->printString("On  ", false);
          else              _disp->printString("OFF ", false);
          if (event == CLICK_UP_2 || event == CLICK_DOWN_2) out2_Enabled = !out2_Enabled;
          break;
      }
      return false; 
    }
};
#endif
