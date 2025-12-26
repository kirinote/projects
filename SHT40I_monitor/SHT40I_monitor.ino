/*
 * SHT40I data monitor with Adafruit_IO
 * This work is hereby placed into the public domain.
 */

#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP 60 // Wake Up/Publish interval: Sec
#include "config.h"
#include <Wire.h>
#include <SHT40.h>

SHT40 sht40;
AdafruitIO_Feed *temperature1 = io.feed("temperature1");
AdafruitIO_Feed *humidity1 = io.feed("humidity1");

void setup() {
//  Serial.begin(115200);
  delay(1000);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
//  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

  // --- Wake Up --- //

  float temp;
  float humi;

  Wire.begin();
  sht40.begin(&Wire, 0x44);
  sht40.measure(temp, humi);

//  Serial.print("Connecting to Adafruit IO");
  io.connect();

  while (io.status() < AIO_CONNECTED) {
//    Serial.print(".");
    delay(500);
  }

//  Serial.println();
//  Serial.println(io.statusText());
  io.run();

//  Serial.print("sending temp -> ");
//  Serial.println(temp);
  temperature1->save(temp);

//  Serial.print("sending humi -> ");
//  Serial.println(humi);
  humidity1->save(humi);

  // --- Deep Sleep --- //

//  Serial.println("Going to sleep now");
//  Serial.flush();
  esp_deep_sleep_start();
//  Serial.println("This will never be printed");
}

void loop() {
  //This is not going to be called
}
