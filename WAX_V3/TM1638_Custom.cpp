#include "TM1638_Custom.h"

// --- ФИНАЛЬНАЯ КАРТА ФИЗИЧЕСКИХ ПОЗИЦИЙ ПОД ВАШУ ПЛАТУ ---
// Переводит логический порядок (0,1,2,3 - левый экран, 4,5,6,7 - правый)
// в физические адреса TM1638 (0 -> 0xC0, 1 -> 0xC2, 2 -> 0xC4 и т.д.)
static const uint8_t POSITION_MAP[] = {
  6, 2, 3, 4,  // Левый экран (соответствует цифрам 1-1, 1-2, 1-3, 1-4)
  7, 1, 0, 5   // Правый экран (соответствует цифрам 2-1, 2-2, 2-3, 2-4)
};


// --- ВАШ ЛИЧНЫЙ ДВОИЧНЫЙ ШРИФТ ---
// Биты идут слева направо: [Бит 7, 6, 5, 4, 3, 2, 1, 0]
// Ставьте '1' там, где палочка должна гореть, и '0' там, где должна гаснуть.
static const uint8_t FLIPPED_FONT[] = {
  0b11101011, // 0
  0b01000001, // 1
  0b10110011, // 2
  0b01110011, // 3
  0b01011001, // 4
  0b01111010, // 5
  0b11111010, // 6
  0b01000011, // 7
  0b11111011, // 8
  0b01111011, // 9
  0b11011011, // 10 (Буква 'A')
  0b11111000, // 11 (Буква 'b')
  0b10101010, // 12 (Буква 'C')
  0b11110001, // 13 (Буква 'd')
  0b10111010, // 14 (Буква 'E')
  0b10011010, // 15 (Буква 'F')
  0b00010000, // 16 (Прочерк '-')
  0b10010000, // 17 (Буква 'r')
  0b00000100, // 18 (Буква '*')
  0b00000000, // 19 (Пустота / Пробел)
  0b10101110, // 20 (Знак '°C')
//  0b10110100, // 20 (Знак '°C')
  0b11101011, // 21 (Буква 'O')
  0b11010000, // 22 (Буква 'n')
  0b10111000, // 23 (Буква 't')
  0b00100000, // 24 (Знак '_')
  0b00000010, // 25 (Знак '~')
  0b01111010, // 26 (Буква 'S')
  0b10110100, // 27 (Буква ''s')
  0b11110000, // 28 (Буква ''o')
  0b11101001, // 29 (Буква 'U')
  0b11100000, // 30 (Буква 'u')
  
};



TM1638_Custom::TM1638_Custom(uint8_t dio, uint8_t clk, uint8_t stb) {
  _pinDio = dio; _pinClk = clk; _pinStb = stb;
}

void TM1638_Custom::init(uint8_t brightness) {
  pinMode(_pinDio, OUTPUT);
  pinMode(_pinClk, OUTPUT);
  pinMode(_pinStb, OUTPUT);
  digitalWrite(_pinStb, HIGH);

  sendCmd(0x40); // Режим фиксированного адреса
  sendCmd(0x88 | (brightness & 0x07)); // Включить + яркость
  clear();
}

void TM1638_Custom::sendCmd(uint8_t cmd) {
  digitalWrite(_pinStb, LOW);
  writeByte(cmd);
  digitalWrite(_pinStb, HIGH);
}

void TM1638_Custom::writeByte(uint8_t data) {
  shiftOut(_pinDio, _pinClk, LSBFIRST, data);
}

void TM1638_Custom::clear() {
  for (uint8_t i = 0; i < 16; i++) {
    digitalWrite(_pinStb, LOW);
    writeByte(0xC0 | i);
    writeByte(0x00);
    digitalWrite(_pinStb, HIGH);
  }
}

// Отправка сырых байт сегментов с учетом карты разводки
void TM1638_Custom::sendRawSegments(uint8_t logicalPos, uint8_t segments) {
  if (logicalPos > 7) return;
  uint8_t physicalPos = POSITION_MAP[logicalPos]; // Переводим логику в физику платы
  
  digitalWrite(_pinStb, LOW);
  writeByte(0xC0 | (physicalPos << 1)); // Четные адреса памяти TM1638
  writeByte(segments);
  digitalWrite(_pinStb, HIGH);
}

// Вывод перевернутой цифры (0-9)
void TM1638_Custom::printDigit(uint8_t logicalPos, uint8_t digit, bool dot) {
  if (digit > 9) return;
  uint8_t segs = FLIPPED_FONT[digit];
  if (dot) segs |= 0x80;
  sendRawSegments(logicalPos, segs);
}

// Вывод текста (например "0FF" или "Err")
void TM1638_Custom::printString(const char* str, bool isLeftScreen) {
  uint8_t start = isLeftScreen ? 0 : 4;
  for (uint8_t i = 0; i < 4; i++) {
    char c = str[i];
    if (c == '\0') {
      for(uint8_t j=i; j<4; j++) sendRawSegments(start + j, 0x00);
      break;
    }
    uint8_t fontIdx = 19; // По умолчанию пробел
    if (c >= '0' && c <= '9') fontIdx = c - '0';
    else if (c == 'A') fontIdx = 10;
    else if (c == 'b') fontIdx = 11;
    else if (c == 'C') fontIdx = 12;
    else if (c == 'd') fontIdx = 13;
	else if (c == 'E') fontIdx = 14;
	else if (c == 'F') fontIdx = 15;
	else if (c == '-') fontIdx = 16;
	else if (c == 'r') fontIdx = 17;
	else if (c == '*') fontIdx = 18;
	else if (c == 'c') fontIdx = 20;
	else if (c == 'O') fontIdx = 21;
	else if (c == 'n') fontIdx = 22;
	else if (c == 't') fontIdx = 23;
	else if (c == '_') fontIdx = 24;
	else if (c == '~') fontIdx = 25;
	else if (c == 'S') fontIdx = 26;
	else if (c == 's') fontIdx = 27;
	else if (c == 'o') fontIdx = 28;
	else if (c == 'U') fontIdx = 29;
    else if (c == 'u') fontIdx = 30;
	
    sendRawSegments(start + i, FLIPPED_FONT[fontIdx]);
  }
}

// Вывод трехзначного числа мощности с символом °C на конце (" 50c", "100c")
void TM1638_Custom::printPower(int num, bool isLeftScreen) {
  uint8_t start = isLeftScreen ? 0 : 4;
  int h = num / 100;
  int t = (num % 100) / 10;
  int o = num % 10;

  // 1-й разряд: сотни (если 0 — гасим)
  sendRawSegments(start,     (h == 0) ? 0x00 : FLIPPED_FONT[h]);
  
  // 2-й разряд: десятки (если сотен и десятков нет — гасим)
  sendRawSegments(start + 1, (h == 0 && t == 0) ? 0x00 : FLIPPED_FONT[t]);
  
  // 3-й разряд: единицы (горят всегда)
  sendRawSegments(start + 2, FLIPPED_FONT[o]);
  
  // 4-й разряд: ВМЕСТО 0x00 отправляем индекс 20 (ваш совмещенный знак °C)
  sendRawSegments(start + 3, FLIPPED_FONT[20]); 
}


// Чтение кнопок панели
unsigned long TM1638_Custom::readButtons() {
  digitalWrite(_pinStb, LOW);
  writeByte(0x42);
  pinMode(_pinDio, INPUT_PULLUP);
  delayMicroseconds(1);
  
  byte b1 = shiftIn(_pinDio, _pinClk, LSBFIRST);
  byte b2 = shiftIn(_pinDio, _pinClk, LSBFIRST);
  byte b3 = shiftIn(_pinDio, _pinClk, LSBFIRST);
  byte b4 = shiftIn(_pinDio, _pinClk, LSBFIRST);
  
  pinMode(_pinDio, OUTPUT);
  digitalWrite(_pinStb, HIGH);
  
  return ((unsigned long)b4 << 24) | ((unsigned long)b3 << 16) | ((byte)b2 << 8) | b1;
}
