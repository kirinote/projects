/*
 * MSP3526 LovyanGFX Display Test
 * for Raspberry Pi Pico 2
 *
 * MSP3526.h
 *
 * Adafruit_GFX ORIGINAL LIBRARY LICENSE:
 * Software License Agreement (BSD License)
 * Copyright (c) 2012 Adafruit Industries.
 * All rights reserved.
 *
 * TFT_eSPI ORIGINAL LIBRARY LICENSE:
 * Software License Agreement (FreeBSD License)
 * Copyright (c) 2020 Bodmer (https://github.com/Bodmer)
 * All rights reserved.
 *
 * LovyanGFX ORIGINAL LIBRARY LICENSE:
 * Software License Agreement (FreeBSD License)
 * Copyright (c) 2020 lovyan03 (https://github.com/lovyan03)
 * All rights reserved.
 */

#ifndef MSP3526_H
#define MSP3526_H

#include <LovyanGFX.hpp>

#define SPI1_MISO 12
#define SPI1_CLK 14
#define SPI1_MOSI 15

#define TFT_CS 13
#define TFT_DC 8
#define TFT_RST 10
#define TFT_BL 11

#define SD_CS 9
#define CTP_RST 7
#define CTP_SDA 4
#define CTP_SCL 5
#define CTP_INT 6

class LGFX_Pico2_MSP3526 : public lgfx::LGFX_Device {
  lgfx::Panel_ST7796 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Touch_FT5x06 _touch_instance;

public:
  LGFX_Pico2_MSP3526(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = 1;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 20000000;
      cfg.pin_sclk = SPI1_CLK;
      cfg.pin_mosi = SPI1_MOSI;
      cfg.pin_miso = SPI1_MISO;
      cfg.pin_dc = TFT_DC;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = TFT_CS;
      cfg.pin_rst = TFT_RST;
      cfg.pin_busy = -1;
      cfg.panel_width = 320;
      cfg.panel_height = 480;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 2;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = false;
      cfg.invert = true;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      cfg.memory_width = 320;
      cfg.memory_height = 480;
      _panel_instance.config(cfg);
    }
    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = TFT_BL;
      cfg.invert = false;
      cfg.freq = 40000;
      cfg.pwm_channel = -1;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }
    {
      auto cfg = _touch_instance.config();
      cfg.x_min = 0;
      cfg.x_max = 319;
      cfg.y_min = 0;
      cfg.y_max = 479;
      cfg.pin_int = CTP_INT;
      cfg.bus_shared = false;
      cfg.offset_rotation = 0;
      cfg.i2c_port = 0;
      cfg.i2c_addr = 0x38;
      cfg.pin_sda = CTP_SDA;
      cfg.pin_scl = CTP_SCL;
      cfg.freq = 400000;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }
    setPanel(&_panel_instance);
  }
};

#endif
