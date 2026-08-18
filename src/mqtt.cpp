#include "mqtt.h"
#include <Arduino.h>
#include "app_data.h"
#include <ArduinoJson.h>
#include "sd_manager.h"



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

    if (mqttClient.connect("WetterstationESP32",
                            "weather/status",
                            0,
                            true,
                            "offline"))
    {
        Serial.println("MQTT connected");

        mqttClient.publish(
            "weather/status",
            "online",
            true
        );

        sendIP();
    }
}

void sendIP()
{
    mqttClient.publish(
        "weather/ip",
        WiFi.localIP().toString().c_str()
    );
}

// void sendLiveData(const AppData& app)
// {
//     static unsigned long lastPublish = 0;

//     if (millis() - lastPublish < 10000)
//         return;
//     lastPublish = millis();

//     JsonDocument doc;

//     doc["temp"] = app.temp;
//     doc["humidity"] = app.humidity;
//     doc["pressure"] = app.pressure;
//     doc["uptime"] = millis() / 1000;
//     doc["wifi"] = app.wifiConnected;
//     doc["ip"] = WiFi.localIP().toString();
//     char timeStr[6];
//     snprintf(timeStr, sizeof(timeStr), "%02d:%02d", app.hour, app.minute);
//     doc["time"] = timeStr;
//     doc["tendency"] = app.weatherTendency;


//     char payload[256];
//     serializeJson(doc, payload);

//     mqttClient.publish(
//         "weather/live",
//         payload
//     );
// }

void sendLiveData(const AppData& app)
{
    // =========================
    // Lokale Mittelung alle 2 s
    // =========================

    static unsigned long lastValue = 0;

    static float tempSum = 0;
    static float humiditySum = 0;
    static float pressureSum = 0;

    static int sampleCount = 0;


    if (millis() - lastValue >= 2000)
    {
        lastValue = millis();

        tempSum += app.temp;
        humiditySum += app.humidity;
        pressureSum += app.pressure;

        sampleCount++;
    }


    // =========================
    // MQTT alle 60 s
    // =========================

    static unsigned long lastPublish = 0;

    if (millis() - lastPublish < 60000)
        return;

    lastPublish = millis();


    // Sicherheitsprüfung
    if (sampleCount == 0)
        return;


    // Mittelwerte berechnen

    float meanTemp =
        tempSum / sampleCount;

    float meanHumidity =
        humiditySum / sampleCount;

    float meanPressure =
        pressureSum / sampleCount;


        // =========================
        // SD-Karte
        // =========================

            sd_saveData(
                app,
                meanTemp,
                meanHumidity,
                meanPressure
            );


    // =========================
    // MQTT-Paket
    // =========================

    JsonDocument doc;

    doc["temp"] = meanTemp;
    doc["humidity"] = meanHumidity;
    doc["pressure"] = meanPressure;

    doc["uptime"] =
        millis() / 1000;

    doc["wifi"] =
        app.wifiConnected;

    doc["ip"] =
        WiFi.localIP().toString();


    char timeStr[6];

    snprintf(
        timeStr,
        sizeof(timeStr),
        "%02d:%02d",
        app.hour,
        app.minute
    );

    doc["time"] = timeStr;

    doc["tendency"] =
        app.weatherTendency;


    char payload[256];

    serializeJson(
        doc,
        payload
    );

    if (mqttClient.connected())
    {
        mqttClient.publish(
        "weather/live",
        payload);
    }
    
    // =========================
    // Mittelung zurücksetzen
    // =========================

    tempSum = 0;
    humiditySum = 0;
    pressureSum = 0;

    sampleCount = 0;
}