#include "mqtt.h"
#include <Arduino.h>
#include "app_data.h"
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void mqtt_init()
{
    mqttClient.setServer(
        MQTT_SERVER,
        MQTT_PORT
    );
}

void mqtt_connect()
{
    if (mqttClient.connected())
        return;

    Serial.println("MQTT connecting...");

    if (mqttClient.connect("WetterstationESP32"))
    {
        Serial.println("MQTT connected");

        mqttClient.publish(
            "weather/status",
            "online", true
        );

        sendIP();
    }
}

void sendTemperature(const AppData& app) {
    static unsigned long lastPublish = 0;

    if (millis() - lastPublish > 10000)
    {
        lastPublish = millis();

        char payload[16];

        snprintf(
            payload,
            sizeof(payload),
            "%.1f",
            app.temp
        );

        mqttClient.publish(
            "weather/temp",
            payload
        );
    }
}

void sendHumidity(const AppData& app) {
    static unsigned long lastPublish = 0;

    if (millis() - lastPublish > 10000)
    {
        lastPublish = millis();

        char payload[16];

        snprintf(
            payload,
            sizeof(payload),
            "%.1f",
            app.humidity
        );

        mqttClient.publish(
            "weather/humidity",
            payload
        );
    }
}
void sendPressure(const AppData& app) {
    static unsigned long lastPublish = 0;

    if (millis() - lastPublish > 10000)
    {
        lastPublish = millis();

        char payload[16];

        snprintf(
            payload,
            sizeof(payload),
            "%.1f",
            app.pressure
        );

        mqttClient.publish(
            "weather/pressure",
            payload
        );
    }
}

void sendUptime()
{
    static unsigned long lastPublish = 0;

    if (millis() - lastPublish < 10000)
        return;

    lastPublish = millis();

    char payload[16];

    snprintf(
        payload,
        sizeof(payload),
        "%lu s",
        millis() / 1000
    );

    mqttClient.publish(
        "weather/uptime",
        payload
    );
}

void sendIP()
{
    mqttClient.publish(
        "weather/ip",
        WiFi.localIP().toString().c_str()
    );
}

void sendLiveData(const AppData& app)
{
    static unsigned long lastPublish = 0;
    if (millis() - lastPublish < 10000)
        return;
    lastPublish = millis();

    JsonDocument doc;

    doc["temp"] = app.temp;
    doc["humidity"] = app.humidity;
    doc["pressure"] = app.pressure;
    doc["uptime"] = millis() / 1000;
    doc["wifi"] = app.wifiConnected;
    doc["ip"] = WiFi.localIP().toString();
    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", app.hour, app.minute);
    doc["time"] = timeStr;
    doc["tendency"] = app.weatherTendency;


    char payload[256];
    serializeJson(doc, payload);

    mqttClient.publish(
        "weather/live",
        payload
    );
}

void sendHistoryData(const AppData& app)
{
    static unsigned long lastPublish = 0;
    if (millis() - lastPublish < 12000)
        return;
    lastPublish = millis();

    JsonDocument doc;

    doc["temp"] = app.tempAverage;
    doc["humidity"] = app.humidityAverage;
    doc["pressure"] = app.pressureAverage;
    doc["uptime"] = millis() / 1000;
    doc["wifi"] = app.wifiConnected;
    doc["ip"] = WiFi.localIP().toString();
    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", app.hour, app.minute);
    doc["time"] = timeStr;
    doc["tendency"] = app.weatherTendency;


    char payload[256];
    serializeJson(doc, payload);

    mqttClient.publish(
        "weather/live",
        payload, true
    );
}