/*
 * ST7789V3 240 * 280 Display Test
 *  designed for Raspberry Pi Pico 2
 *
 * uc_disp_test.ino
 *
 * Copyright (c) 2026 K. Yoshi
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <LovyanGFX.hpp>
#include <math.h>
#include "fonts.h"

class LGFX_Pico2_SPI_ST7789 : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;

public:
  LGFX_Pico2_SPI_ST7789(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = 1;
      cfg.spi_mode = 0;
      cfg.freq_write = 37500000;
      cfg.freq_read = 18750000;
      cfg.pin_sclk = 14;
      cfg.pin_mosi = 15;
      cfg.pin_miso = -1;
      cfg.pin_dc = 8;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 13;
      cfg.pin_rst = 10;
      cfg.pin_busy = -1;
      cfg.panel_width = 240;
      cfg.panel_height = 280;
      cfg.offset_x = 0;
      cfg.offset_y = 20;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = false;
      cfg.invert = true;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      cfg.memory_width = 240;
      cfg.memory_height = 320;
      _panel_instance.config(cfg);
    }
    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = 11;
      cfg.invert = false;
      cfg.freq = 40000;
      cfg.pwm_channel = 5;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }
    setPanel(&_panel_instance);
  }
};

LGFX_Pico2_SPI_ST7789 display;

/* Original Drawing Functions */
void drawParallelogram(int32_t x, int32_t y, int32_t h, int32_t v, int32_t s, uint32_t color) {
  display.drawFastHLine(x, y, h, color);
  display.drawFastHLine(x + s, y - v, h, color);
  display.drawLine(x, y, x + s, y - v, color);
  display.drawLine(x + h, y, x + s + h, y - v, color);
}

void fillParallelogram(int32_t x, int32_t y, int32_t h, int32_t v, int32_t s, uint32_t color) {
  for (int32_t i = 0; i <= v; i++) {
    int32_t x_offset = (i * s) / v;
    display.drawFastHLine(x + x_offset, y - i, h, color);
  }
}

void drawRadialLine(int32_t x, int32_t y, int32_t r1, int32_t r2, int32_t n, uint32_t color) {
  if (n <= 0) return;
  float angleStep = 2.0f * M_PI / n;
  for (int32_t i = 0; i < n; i++) {
    float angle = (i * angleStep) - (M_PI / 2.0f);
    int32_t x_start = x + (int32_t)(cosf(angle) * r2);
    int32_t y_start = y + (int32_t)(sinf(angle) * r2);
    int32_t x_end = x + (int32_t)(cosf(angle) * r1);
    int32_t y_end = y + (int32_t)(sinf(angle) * r1);
    display.drawLine(x_start, y_start, x_end, y_end, color);
  }
}

void drawLinearScale(int32_t x, int32_t y, int32_t r, int32_t s, int32_t n, float d, uint32_t color) {
  if (n <= 0) return;
  float rad = d * M_PI / 180.0f;
  float cos_dir = cosf(rad);
  float sin_dir = sinf(rad);
  float cos_perp = -sin_dir;
  float sin_perp = cos_dir;
  float step = (2.0f * r) / n;
  for (int32_t i = 0; i <= n; i++) {
    float dist = -r + (i * step);
    float cx = x + (dist * cos_dir);
    float cy = y + (dist * sin_dir);
    int32_t tick_x1 = cx + (s * cos_perp);
    int32_t tick_y1 = cy + (s * sin_perp);
    int32_t tick_x2 = cx - (s * cos_perp);
    int32_t tick_y2 = cy - (s * sin_perp);
    display.drawLine(tick_x1, tick_y1, tick_x2, tick_y2, color);
  }
}

void drawFourRoundCorners(int32_t x, int32_t y, int32_t h, int32_t v, int32_t a, int32_t b, int32_t r, int32_t w, uint32_t color) {
  if (w <= 0) return;
  int32_t x0 = x - (h / 2);
  int32_t x1 = x + (h / 2);
  int32_t y0 = y - (v / 2);
  int32_t y1 = y + (v / 2);
  if (a < r) a = r;
  if (b < r) b = r;
  int32_t r_in = (r > w) ? (r - w) : 0;
  display.fillRect(x0 + r, y0, a - r, w, color);
  display.fillRect(x0, y0 + r, w, b - r, color);
  if (r > 0) display.fillArc(x0 + r, y0 + r, r_in, r, 180, 270, color);
  display.fillRect(x1 - a, y0, a - r, w, color);
  display.fillRect(x1 - w, y0 + r, w, b - r, color);
  if (r > 0) display.fillArc(x1 - r, y0 + r, r_in, r, 270, 360, color);
  display.fillRect(x0 + r, y1 - w, a - r, w, color);
  display.fillRect(x0, y1 - b, w, b - r, color);
  if (r > 0) display.fillArc(x0 + r, y1 - r, r_in, r, 90, 180, color);
  display.fillRect(x1 - a, y1 - w, a - r, w, color);
  display.fillRect(x1 - w, y1 - b, w, b - r, color);
  if (r > 0) display.fillArc(x1 - r, y1 - r, r_in, r, 0, 90, color);
}

void setup(void) {
  display.setBrightness(0);
  display.init();
  display.setRotation(0);
  delay(100);

  display.setColorDepth(24);
  display.setTextSize(2);
  display.setTextColor(0x4444CCU);
  display.fillScreen(TFT_BLACK);

  display.startWrite();

  display.drawCircle(120, 140, 80, 0x333399U);
  display.fillCircle(120, 140, 79, 0x000009U);
  display.fillCircle(120, 140, 55, 0x000000U);

  display.loadFont(michroma_12);
  display.drawCenterString("DENSITY", 120, 235);
  display.unloadFont();

  display.loadFont(bebasneue_20);
  display.drawCenterString("86%", 190, 20);
  display.unloadFont();

  fillParallelogram(20, 40, 60, 12, 5, 0x333399U);
  fillParallelogram(85, 40, 5, 12, 5, 0x333399U);
  fillParallelogram(95, 40, 5, 12, 5, 0x333399U);

  drawRadialLine(120, 140, 80, 70, 12, 0x333399U);
  drawRadialLine(120, 140, 40, 20, 4, 0x333399U);

  drawLinearScale(120, 110, 70, 2, 10, 0, 0x333399U);
  drawLinearScale(120, 170, 70, 2, 10, 0, 0x333399U);
  drawLinearScale(30, 140, 70, 2, 10, 90, 0x333399U);
  drawLinearScale(210, 140, 70, 2, 10, 90, 0x333399U);

  drawFourRoundCorners(120, 140, 60, 30, 20, 10, 8, 1, 0x333399U);

  display.endWrite();

  for (int i = 0; i <= 255; i += 5) {
    display.setBrightness(i);
    delay(50);
  }
}

void loop(void) {
}
