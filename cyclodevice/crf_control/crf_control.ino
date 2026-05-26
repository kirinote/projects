/*
 * Cycloidal Rotor Fan Control
 * with D3536-750KV & 28BYJ-48
 *
 * ESC: SKYWALKER 50A-UBEC
 * Stepper Motor Driver: ULN2003A
 *
 * Arduino Uno R3 Pinout
 *   A0  VR1 10kOhm as JOG
 *   A1  VR2 10kOhm as VOL
 *   D6  Pulse out to White wire
 *   D8  IN1
 *   D9  IN2
 *   D10 IN3
 *   D11 IN4
 *   D18 SDA
 *   D19 SCL
 *
 * This work is hereby placed into the public domain.
 */

#include <Wire.h>
#include <Servo.h>
#include <Stepper.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define PULSE_MAX 2000
#define PULSE_MIN 1000
#define STEPS_PER_REV 2048
#define STEPM_RPM 10
#define ESC_PIN 6
#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11
#define SCREEN_W 128
#define SCREEN_H 32

Servo esc;
Stepper stepper(STEPS_PER_REV, IN1, IN3, IN2, IN4);
Adafruit_SSD1306 oled(SCREEN_W, SCREEN_H, &Wire, -1);

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ESC_PIN, OUTPUT);

  stepper.setSpeed(STEPM_RPM);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;)
      ;
  }

  oled.setTextSize(2);
  oled.setTextColor(SSD1306_WHITE);
  oled.clearDisplay();

  while (!(analogRead(A0) == 0 && analogRead(A1) == 0)) {
    oled.setCursor(0, 12);
    oled.print("Turn Down!");
    oled.display();
  }

  while (analogRead(A0) < 512) {
    oled.clearDisplay();
    oled.setCursor(0, 12);
    oled.print("JOG Up!");
    oled.display();
  }

  esc.attach(ESC_PIN);
  delay(1);
  esc.writeMicroseconds(PULSE_MAX);
  oled.clearDisplay();
  oled.setCursor(0, 12);
  oled.print("Thrtl:Max");
  oled.display();
  delay(2000);

  esc.writeMicroseconds(PULSE_MIN);
  oled.clearDisplay();
  oled.setCursor(0, 12);
  oled.print("Thrtl:Min");
  oled.display();
  delay(2000);

  oled.clearDisplay();
  oled.setTextSize(1);
}

void loop() {
  int jog = analogRead(A0);
  int vol = analogRead(A1);

  oled.clearDisplay();
  oled.setCursor(12, 10);
  oled.print("JOG ");
  oled.print(jog);
  oled.setCursor(72, 10);
  oled.print("VOL ");
  oled.print(vol);

  if (jog >= 0 && jog <= 340) {
    stepper.step(STEPS_PER_REV / 16);
  } else if (jog >= 341 && jog <= 681) {
    stopMotor();
  } else if (jog >= 682 && jog <= 1023) {
    stepper.step(STEPS_PER_REV / -16);
  }

  oled.setCursor(12, 24);
  oled.print("Width ");
  oled.print(mapInput(vol));
  oled.print(" uSec");
  oled.display();
  esc.writeMicroseconds(mapInput(vol));
}

void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

int mapInput(int input) {
  return map(input, 0, 1023, PULSE_MIN, PULSE_MAX);
}
