//#include <EEPROM.h>
#include "TM1638_Custom.h"
#include "Buttons.h"
#include "Menu.h"

TM1638_Custom disp(8, 7, 4);
ButtonHandler btn(&disp);
MenuHandler menu(&disp);

const int HEAT_CH1 = 5; const int HEAT_CH2 = 6;
const int PROT_CH1 = 2; const int PROT_CH2 = 3;

int powerCh1 = 50, powerCh2 = 50;
bool stateCh1 = false, stateCh2 = false;
bool turboActive = false;
unsigned long turboStartTime = 0;
int savedPowerCh1 = 0;
volatile bool errorCh1 = false, errorCh2 = false;
unsigned long lastBtnTime = 0;

// Глобальные переменные настроек
int minTemp1, maxTemp1, minTemp2, maxTemp2;
int turboTime; 
int turboTemp; 
bool SOUND;
bool out1_Enabled; // Переменная активности левого канала
bool out2_Enabled; // Переменная активности правого канала
bool blinkState = false; 

byte isMenuMode = 0; 
unsigned long menuSetStartTime = 0; 

void isr_Emergency_Ch1() { if (out1_Enabled) { digitalWrite(HEAT_CH1, LOW); errorCh1 = true; } }
void isr_Emergency_Ch2() { if (out2_Enabled) { digitalWrite(HEAT_CH2, LOW); errorCh2 = true; } }

void loadSettings() {
  powerCh1 = EEPROM.read(0); if (powerCh1 > 200) powerCh1 = 50;
  powerCh2 = EEPROM.read(1); if (powerCh2 > 200) powerCh2 = 50;
  minTemp1 = EEPROM.read(2); if (minTemp1 > 200 || minTemp1 < 10) minTemp1 = 50;
  maxTemp1 = EEPROM.read(3); if (maxTemp1 > 200 || maxTemp1 < 50) maxTemp1 = 200;
  minTemp2 = EEPROM.read(4); if (minTemp2 > 200 || minTemp2 < 10) minTemp2 = 50;
  maxTemp2 = EEPROM.read(5); if (maxTemp2 > 200 || maxTemp2 < 50) maxTemp2 = 200;
  
  turboTime = (EEPROM.read(6) << 8) | EEPROM.read(7);
  if (turboTime > 999 || turboTime == 0) turboTime = 10;
  
  turboTemp = EEPROM.read(8); if (turboTemp > 200 || turboTemp < 50) turboTemp = 200;
  SOUND = (EEPROM.read(9) == 0) ? false : true;

  // Чиркаем из памяти состояние активности контроллеров
  out1_Enabled = (EEPROM.read(10) == 0) ? false : true;
  out2_Enabled = (EEPROM.read(11) == 0) ? false : true;
}

void setup() {
  disp.init(5); 
  pinMode(HEAT_CH1, OUTPUT); pinMode(HEAT_CH2, OUTPUT);
  digitalWrite(HEAT_CH1, LOW); digitalWrite(HEAT_CH2, LOW);
  
  pinMode(PROT_CH1, INPUT_PULLUP); pinMode(PROT_CH2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PROT_CH1), isr_Emergency_Ch1, FALLING);
  attachInterrupt(digitalPinToInterrupt(PROT_CH2), isr_Emergency_Ch2, FALLING);
  
  loadSettings();
}

void loop() {
  static unsigned long lastBlinkTime = 0;
  if (millis() - lastBlinkTime >= 250) { blinkState = !blinkState; lastBlinkTime = millis(); }

  ButtonEvent event = btn.checkButtons();

  if (isMenuMode == 1) { // Показ "SEt" (Вход в меню разрешен всегда)
    disp.printString("SEt ", true); disp.printString("    ", false);
    if (millis() - menuSetStartTime >= 2000) { isMenuMode = 2; menu.reset(); }
  }
  else if (isMenuMode == 2) { // Работа внутри меню
    if (menu.handleMenu(event)) { isMenuMode = 0; }
  }
  else { // Обычная работа шпателя
    if (event == HOLD_TURBO) { isMenuMode = 1; menuSetStartTime = millis(); return; }

    // --- ЛОГИКА ТУРБО (работает, только если левый канал вообще активен) ---
    if (out1_Enabled && turboActive && (millis() - turboStartTime >= ((unsigned long)turboTime * 1000))) {
      turboActive = false; powerCh1 = savedPowerCh1;
    }

    // --- СИЛОВАЯ ЧАСТЬ И ОТРИСОВКА ЭКРАНОВ ---
    
    // ЛЕВЫЙ КАНАЛ (КАНАЛ 1)
    if (out1_Enabled) {
      if (errorCh1)      { digitalWrite(HEAT_CH1, LOW); }
      else if (stateCh1) { analogWrite(HEAT_CH1, map(powerCh1, 0, 200, 0, 255)); }
      else               { digitalWrite(HEAT_CH1, LOW); }

      if (errorCh1) { if (blinkState) disp.printString("Err ", true); else disp.printString("    ", true); }
      else if (turboActive) { if (blinkState) disp.printString("trbO", true); else disp.printPower(powerCh1, true); }
      else if (stateCh1) { disp.printPower(powerCh1, true); }
      else               { disp.printString("OFF ", true); }
    } else {
      // КАНАЛ ВЫКЛЮЧЕН В МЕНЮ: полностью гасим силу и экран
      digitalWrite(HEAT_CH1, LOW);
      disp.printString("    ", true);
    }

    // ПРАВЫЙ КАНАЛ (КАНАЛ 2)
    if (out2_Enabled) {
      if (errorCh2)      { digitalWrite(HEAT_CH2, LOW); }
      else if (stateCh2) { analogWrite(HEAT_CH2, map(powerCh2, 0, 200, 0, 255)); }
      else               { digitalWrite(HEAT_CH2, LOW); }

      if (errorCh2) { if (blinkState) disp.printString("Err ", false); else disp.printString("    ", false); }
      else if (stateCh2) { disp.printPower(powerCh2, false); }
      else               { disp.printString("OFF ", false); }
    } else {
      // КАНАЛ ВЫКЛЮЧЕН В МЕНЮ: полностью гасим силу и экран
      digitalWrite(HEAT_CH2, LOW);
      disp.printString("    ", false);
    }

    // --- ОБРАБОТКА КНОПОК РАБОТЫ (ИГНОРИРУЕМ ТЕ, ЧТО ОТКЛЮЧЕНЫ) ---
    // Кнопки левого канала
    if (out1_Enabled) {
      if (event == CLICK_ONOFF_1) { if (errorCh1) errorCh1 = false; else stateCh1 = !stateCh1; }
      if (stateCh1 && !turboActive && !errorCh1) {
        if ((event == CLICK_UP_1 || event == HOLD_UP_1) && powerCh1 < maxTemp1) { powerCh1 += 5; EEPROM.write(0, powerCh1); }
        if ((event == CLICK_DOWN_1 || event == HOLD_DOWN_1) && powerCh1 > minTemp1) { powerCh1 -= 5; EEPROM.write(0, powerCh1); }
      }
      if (event == CLICK_TURBO && stateCh1 && !turboActive && !errorCh1) {
        turboActive = true; turboStartTime = millis(); savedPowerCh1 = powerCh1; powerCh1 = turboTemp;
      }
    }

    // Кнопки правого канала
    if (out2_Enabled) {
      if (event == CLICK_ONOFF_2) { if (errorCh2) errorCh2 = false; else stateCh2 = !stateCh2; }
      if (stateCh2 && !errorCh2) {
        if ((event == CLICK_UP_2 || event == HOLD_UP_2) && powerCh2 < maxTemp2) { powerCh2 += 5; EEPROM.write(1, powerCh2); }
        if ((event == CLICK_DOWN_2 || event == HOLD_DOWN_2) && powerCh2 > minTemp2) { powerCh2 -= 5; EEPROM.write(1, powerCh2); }
      }
    }
  }
  delay(10);
}
