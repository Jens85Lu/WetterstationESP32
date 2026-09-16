#include <Arduino.h>

#include "led.h"

#define LED_BUILTIN 2

void led_init() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void led_on() {
  digitalWrite(LED_BUILTIN, HIGH);
}

void led_off() {
  digitalWrite(LED_BUILTIN, LOW);
}