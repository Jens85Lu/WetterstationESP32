// OLED initialisieren und low level Displayfunktionen
#include "display.h"
#include <Wire.h>
#include <U8g2lib.h>
#include <Arduino.h>
#include "app_data.h"
#include "weather_ui.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, 
    U8X8_PIN_NONE);

void display_init() {
  Wire.begin(21, 22);
  display.begin();
  display.clearBuffer();
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawStr(0,20, "Hello ESP32!");
  display.sendBuffer();
  Serial.println("Display OK");
}

void display_showText(const char* text) {
  display.clearBuffer();
  display.setFont(u8g2_font_6x12_tf);
  display.drawStr(0, 30, text);
  display.sendBuffer();
}

void display_update() {

  weather_show(app);

}