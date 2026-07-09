#pragma once

#include <WiFi.h>
#include <PubSubClient.h>
#include "app_data.h"

#define MQTT_SERVER "192.168.0.19"
#define MQTT_PORT 1883


extern WiFiClient espClient;
extern PubSubClient mqttClient;

void mqtt_init();
void mqtt_connect();

void sendTemperature(const AppData& app);
void sendHumidity(const AppData& app);
void sendPressure(const AppData& app);

void sendUptime();
void sendIP();
void sendLiveData(const AppData& app);
void sendHistoryData(const AppData& app);