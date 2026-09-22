#ifndef TM1638_CUSTOM_H
#define TM1638_CUSTOM_H

#include <Arduino.h>

class TM1638_Custom {
  private:
    uint8_t _pinDio;
    uint8_t _pinClk;
    uint8_t _pinStb;

    void sendCmd(uint8_t cmd);
    void writeByte(uint8_t data);

  public:
    TM1638_Custom(uint8_t dio, uint8_t clk, uint8_t stb);
    void init(uint8_t brightness = 4);
    void clear();
    void sendRawSegments(uint8_t logicalPos, uint8_t segments);
    void printDigit(uint8_t logicalPos, uint8_t digit, bool dot = false);
    void printString(const char* str, bool isLeftScreen);
    void printPower(int num, bool isLeftScreen);
    unsigned long readButtons();
};

#endif
