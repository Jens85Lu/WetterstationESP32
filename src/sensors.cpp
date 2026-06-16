#include "sensors.h"
#include "dht_sensor.h"
#include "bmp280.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>

static float temp;
static float hum;
static float press;
static float bmpTemperature;

void sensors_init() {
    dht_init();
    Serial.println("DHT22 started");
    if (!bmp280_init()) {
        Serial.println("BMP280 not found!");
        while (1);
    }
    Serial.println("BMP280 OK");
}

void sensors_read() {
    temp = dht_getTemperature();
    hum = dht_getHumidity();
    press = bmp280_getPressure();
    bmpTemperature = bmp280_getTemperature();
}

float getTemp() {return temp;}

float getHumidity() {return hum;}

float getPressure() {return press;}

float getBmpTemperature() {return bmpTemperature;}
