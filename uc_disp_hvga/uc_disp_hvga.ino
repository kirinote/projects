/*
 * MSP3526 LovyanGFX Display Test
 * for Raspberry Pi Pico 2
 *
 * uc_disp_hvga.ino
 *
 * Copyright (c) 2026 K. Yoshi
 * This source code is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <cstdint>
#include <FS.h>
#include <SD.h>
#include <math.h>
#include "MSP3526.h"
#include "fonts.h"

LGFX_Pico2_MSP3526 display;

const int btnW = 120;
const int btnH = 50;
int btnX = 0;
int btnY = 0;

bool isTouching = false;

void drawWriteButton(bool pressed) {
  uint16_t color = pressed ? TFT_DARKGREY : TFT_BLUE;
  display.fillRoundRect(btnX, btnY, btnW, btnH, 8, color);
  display.drawRoundRect(btnX, btnY, btnW, btnH, 8, TFT_WHITE);
  display.setTextColor(TFT_WHITE);
  display.drawCenterString("Write", btnX + (btnW / 2), btnY + 10);
}

void writeToSD() {
  int centerX = display.width() / 2;
  display.fillRect(0, 160, display.width(), 50, TFT_BLACK);
  SPI1.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));
  SPI1.endTransaction();
  if (!SD.begin(SD_CS, SPI1)) return;
  File myFile = SD.open("/test.txt", "w");
  if (myFile) {
    myFile.println("Hello, Pico 2!");
    myFile.close();
    display.setTextColor(TFT_CYAN);
    display.drawCenterString("Saved: OK!", centerX, 160);
  } else {
    display.setTextColor(TFT_RED);
    display.drawCenterString("Error: Cannot Open", centerX, 175);
  }
}

void setup(void) {
  pinMode(SD_CS, OUTPUT);
  pinMode(CTP_RST, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  digitalWrite(CTP_RST, HIGH);

  SPI1.setRX(SPI1_MISO);
  SPI1.setTX(SPI1_MOSI);
  SPI1.setSCK(SPI1_CLK);

  display.init();
  display.setBrightness(0);
  display.setRotation(1);
  display.setColorDepth(16);
  display.fillScreen(TFT_BLACK);

  display.loadFont(barlow_regular_28);
  int centerX = display.width() / 2;
  btnX = centerX - (btnW / 2);
  btnY = display.height() - btnH - 30;
  display.drawCenterString("TOUCH PANEL TEST", centerX, 50);

  if (!SD.begin(SD_CS, SPI1)) {
    display.setTextColor(TFT_RED);
    display.drawCenterString("SD Init Failed!", centerX, 120);
  } else {
    display.setTextColor(TFT_GREEN);
    display.drawCenterString("SD Card Ready", centerX, 120);
  }
  digitalWrite(SD_CS, HIGH);

  drawWriteButton(false);
  for (int brightness = 0; brightness <= 255; brightness++) {
    display.setBrightness(brightness);
    delay(10);
  }
}

void loop(void) {
  int32_t x, y;

  if (display.getTouch(&x, &y)) {
    if (x >= btnX && x <= (btnX + btnW) && y >= btnY && y <= (btnY + btnH)) {
      if (!isTouching) {
        isTouching = true;
        drawWriteButton(true);
        writeToSD();
      }
    }
  } else {
    if (isTouching) {
      isTouching = false;
      drawWriteButton(false);
    }
  }
}
