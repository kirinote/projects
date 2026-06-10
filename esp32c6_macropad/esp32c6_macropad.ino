/*
 * MacroPad using XIAO ESP32C6
 *
 * esp32c6_macropad.ino
 *
 * Copyright (c) 2026 K. Yoshi
 * This source code is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <Arduino.h>
#include <esp_sleep.h>
#include <esp_pm.h>

// HijelHID_BLEKeyboardとNimBLE-Arduinoをインクルード
#include <HijelHID_BLEKeyboard.h>

// --- ピン定義 ---
const int PIN_ACOPY = D6;  // TTP223 (全選択＆コピー)
const int PIN_PASTE = D7;  // TTP223 (ペースト)
const int PIN_LED = 15;    // USER LED (XIAO ESP32-C6オンボードLED)

// --- BLEキーボードの初期化 ---
HijelHID_BLEKeyboard bleKeyboard("MacroPad C6", "Creator", 100);

void setup() {
  // ピン設定 (TTP223はActive High。浮き防止のためINPUT_PULLDOWN)
  pinMode(PIN_ACOPY, INPUT_PULLDOWN);
  pinMode(PIN_PASTE, INPUT_PULLDOWN);
  pinMode(PIN_LED, OUTPUT);

  // BLEキーボード開始
  bleKeyboard.begin();

  // --- Automatic Light Sleep（自動スリープ）の設定 ---
  gpio_wakeup_enable((gpio_num_t)PIN_ACOPY, GPIO_INTR_HIGH_LEVEL);
  gpio_wakeup_enable((gpio_num_t)PIN_PASTE, GPIO_INTR_HIGH_LEVEL);
  esp_sleep_enable_gpio_wakeup();

  // 電源管理の設定
  esp_pm_config_t pm_config = {
    .max_freq_mhz = 160,
    .min_freq_mhz = 20,
    .light_sleep_enable = true
  };
  esp_pm_configure(&pm_config);
}

void loop() {
  if (bleKeyboard.isConnected()) {
    digitalWrite(PIN_LED, LOW);

    // --- D6: 全選択 (Ctrl+A) & コピー (Ctrl+C) ---
    if (digitalRead(PIN_ACOPY) == HIGH) {
      // 1. まず Ctrl キーを押しっぱなしにする
      bleKeyboard.press(KEY_LCTRL);
      delay(50);

      // 2. Ctrlを押したまま、文字としての 'a' を入力
      bleKeyboard.print("a");
      delay(50);

      // 3. 一度 Ctrl を離す（全選択完了）
      bleKeyboard.releaseAll();
      delay(150);  // Windows側が選択状態を認識するマージン

      // 4. 再び Ctrl キーを押しっぱなしにする
      bleKeyboard.press(KEY_LCTRL);
      delay(50);

      // 5. Ctrlを押したまま、文字としての 'c' を入力
      bleKeyboard.print("c");
      delay(50);

      // 6. すべて離す
      bleKeyboard.releaseAll();

      while (digitalRead(PIN_ACOPY) == HIGH) { delay(10); }
    }

    // --- D7: ペースト (Ctrl+V) ---
    if (digitalRead(PIN_PASTE) == HIGH) {
      // 1. Ctrl キーを押しっぱなしにする
      bleKeyboard.press(KEY_LCTRL);
      delay(50);

      // 2. 文字としての 'v' を入力
      bleKeyboard.print("v");
      delay(50);

      // 3. すべて離す
      bleKeyboard.releaseAll();

      while (digitalRead(PIN_PASTE) == HIGH) { delay(10); }
    }

  } else {
    // ペアリング待機中の点滅処理
    unsigned long currentMillis = millis();
    if ((currentMillis / 500) % 2 == 0) {
      digitalWrite(PIN_LED, HIGH);
    } else {
      digitalWrite(PIN_LED, LOW);
    }
  }

  delay(10);
}
