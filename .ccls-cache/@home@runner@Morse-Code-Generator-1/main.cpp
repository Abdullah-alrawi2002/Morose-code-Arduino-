#include <PS2Keyboard.h>
#include <U8g2lib.h>
#include "Lewis.h"
#include <TimerOne.h>

// Display settings using SW SPI
U8G2_ST7565_ERC12864_1_4W_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 9, 8);
const int dataPin = 3;
const int irqPin = 2;
const int maxCharacters = 10;
const int maxWordsPerMinute = 30;
const int minWordsPerMinute = 1;
String displayLines[2][maxCharacters];
int morseCodes[maxCharacters];
PS2Keyboard keyboard;
Lewis morse;
byte currentSpeed = 7;

void setup() {
  Serial.begin(9600);
  keyboard.begin(dataPin, irqPin);
  u8g2.begin();
  u8g2.setContrast(40);
  morse.begin(7, 7, currentSpeed, true);
  Timer1.initialize(10000);
  Timer1.attachInterrupt(timerInterrupt);
}

void loop() {
  readKeyboard();
  updateDisplay();
}

void readKeyboard() {
  if (keyboard.available()) {
    char c = keyboard.read();
    handleKey(c);
  }
}

void handleKey(char c) {
  switch (c) {
    case PS2_ENTER:
      sendMorse();
      break;
    case PS2_PAGEDOWN:
      adjustSpeed(-1);
      break;
    case PS2_PAGEUP:
      adjustSpeed(1);
      break;
    case PS2_DELETE:
      deleteLastCharacter();
      break;
    default:
      addCharacter(c);
      break;
  }
}

void adjustSpeed(int change) {
  currentSpeed += change;
  currentSpeed = constrain(currentSpeed, minWordsPerMinute, maxWordsPerMinute);
  morse.begin(7, 7, currentSpeed, true);
}

void deleteLastCharacter() {
  if (morseCodes[0] > 0) {
    displayLines[1][morseCodes[0] - 1] = "";
    morseCodes[0]--;
  }
}

void addCharacter(char c) {
  if (morseCodes[0] < maxCharacters) {
    displayLines[1][morseCodes[0]] = String(c);
    morseCodes[morseCodes[0]] = c;
    morseCodes[0]++;
  }
}

void updateDisplay() {
  u8g2.firstPage();
  do {
    drawDisplay();
  } while (u8g2.nextPage());
}

void drawDisplay() {
  u8g2.setFont(u8g2_font_10x20_tr);
  u8g2.drawStr(0, 15, "Morse");
  u8g2.drawStr(75, 15, String(currentSpeed).c_str());
  u8g2.drawStr(98, 15, "wpm");
  for (int i = 0; i < maxCharacters; i++) {
    u8g2.drawStr(i * 10, 40, displayLines[0][i].c_str());
    u8g2.drawStr(i * 10, 60, displayLines[1][i].c_str());
  }
}

void sendMorse() {
  for (int i = 0; i < morseCodes[0]; i++) {
    morse.write(morseCodes[i]);
  }
  clearDisplayLines();
}

void clearDisplayLines() {
  for (int i = 0; i < maxCharacters; i++) {
    displayLines[0][i] = displayLines[1][i];
    displayLines[1][i] = "";
    morseCodes[i] = 0;
  }
  morseCodes[0] = 0;
}

void timerInterrupt() {
  morse.timerISR();
}
