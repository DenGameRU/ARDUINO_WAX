#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>
#include "TM1638_Custom.h"

enum ButtonEvent {
  BTN_NONE = 0,
  CLICK_ONOFF_1, CLICK_DOWN_1, CLICK_UP_1,
  CLICK_ONOFF_2, CLICK_DOWN_2, CLICK_UP_2,
  CLICK_TURBO,   HOLD_TURBO,
  HOLD_UP_1, HOLD_DOWN_1, HOLD_UP_2, HOLD_DOWN_2 // Новые события удержания стрелок
};

class ButtonHandler {
  private:
    TM1638_Custom* _disp;
    unsigned long _lastRawCode;
    unsigned long _pressStartTime;
    unsigned long _lastRepeatTime;
    bool _isHolding;
    bool _hasFiredHoldTurbo;
    
    const unsigned long DEBOUNCE_DELAY = 50;
    const unsigned long HOLD_TURBO_DELAY = 5000; // 5 секунд для меню
    const unsigned long REPEAT_DELAY = 400;     // Через сколько начнет мотать стрелка
    const unsigned long REPEAT_RATE = 100;      // Скорость перемотки (мс)

  public:
    ButtonHandler(TM1638_Custom* disp) {
      _disp = disp; _lastRawCode = 0; _pressStartTime = 0;
      _lastRepeatTime = 0; _isHolding = false; _hasFiredHoldTurbo = false;
    }

    ButtonEvent checkButtons() {
      unsigned long rawCode = _disp->readButtons();
      
      const unsigned long MASK_ONOFF_1 = 0x4;
      const unsigned long MASK_DOWN_1  = 0x40000;
      const unsigned long MASK_UP_1    = 0x400000;
      const unsigned long MASK_ONOFF_2 = 0x400;
      const unsigned long MASK_DOWN_2  = 0x4000000;
      const unsigned long MASK_UP_2    = 0x4000;
      const unsigned long MASK_TURBO   = 0x40;

      unsigned long now = millis();

      if (rawCode != 0 && _lastRawCode == 0) {
        _pressStartTime = now; _lastRepeatTime = now;
        _lastRawCode = rawCode; _isHolding = true; _hasFiredHoldTurbo = false;
        return BTN_NONE;
      }

      if (rawCode != 0 && _lastRawCode == rawCode && _isHolding) {
        // 1. Проверка удержания кнопки TURBO для входа в меню
        if (rawCode == MASK_TURBO && !_hasFiredHoldTurbo) {
          if (now - _pressStartTime >= HOLD_TURBO_DELAY) {
            _hasFiredHoldTurbo = true;
            return HOLD_TURBO;
          }
        }
        // 2. Обработка автоперемотки для стрелок
        if (now - _pressStartTime >= REPEAT_DELAY) {
          if (now - _lastRepeatTime >= REPEAT_RATE) {
            _lastRepeatTime = now;
            if (rawCode == MASK_UP_1)   return HOLD_UP_1;
            if (rawCode == MASK_DOWN_1) return HOLD_DOWN_1;
            if (rawCode == MASK_UP_2)   return HOLD_UP_2;
            if (rawCode == MASK_DOWN_2) return HOLD_DOWN_2;
          }
        }
        return BTN_NONE;
      }

      if (rawCode == 0 && _lastRawCode != 0) {
        unsigned long pressDuration = now - _pressStartTime;
        unsigned long releasedCode = _lastRawCode;
        _lastRawCode = 0; _isHolding = false;

        if (pressDuration < DEBOUNCE_DELAY) return BTN_NONE;
        if (releasedCode == MASK_TURBO && _hasFiredHoldTurbo) { _hasFiredHoldTurbo = false; return BTN_NONE; }
        
        // Если долго мотали стрелку, обычный одиночный клик при отпускании не генерируем
        if (pressDuration >= REPEAT_DELAY) return BTN_NONE;

        if (releasedCode == MASK_ONOFF_1) return CLICK_ONOFF_1;
        if (releasedCode == MASK_DOWN_1)  return CLICK_DOWN_1;
        if (releasedCode == MASK_UP_1)    return CLICK_UP_1;
        if (releasedCode == MASK_ONOFF_2) return CLICK_ONOFF_2;
        if (releasedCode == MASK_DOWN_2)  return CLICK_DOWN_2;
        if (releasedCode == MASK_UP_2)    return CLICK_UP_2;
        if (releasedCode == MASK_TURBO)   return CLICK_TURBO;
      }
      return BTN_NONE;
    }
};
#endif
