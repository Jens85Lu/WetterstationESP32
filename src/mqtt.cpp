#include "mqtt.h"
#include <Arduino.h>
#include "app_data.h"
#include <ArduinoJson.h>
#include "sd_manager.h"
#include "time_manager.h"



WiFiClient espClient;
PubSubClient mqttClient(espClient);

void mqtt_init()
{
    mqttClient.setServer(
        MQTT_SERVER,
        MQTT_PORT
    );
    mqttClient.setBufferSize(4096);
    mqttClient.setCallback(mqtt_callback);
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

        mqttClient.subscribe("weather/history/request");

        mqttClient.publish(
            "weather/status",
            "online",
            true
        );

        sendIP();
        return;
    }
    Serial.println("MQTT connection failed!");
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
    if (!app.timeValid)
    {
        Serial.println(
        "Weltzeit nicht gueltig - Live-Daten uebersprungen."
        );
        return;
    }
    // =========================
    // MQTT-Paket
    // =========================

    JsonDocument doc;

    doc["temp"] = app.tempMean; // Alle 30 Samples à 2 Sekunden = 1 Minute Mittelwert
    doc["humidity"] = app.humidityMean;
    doc["pressure"] = app.pressureMean;

    doc["uptime"] =
        millis() / 1000;

    doc["wifi"] =
        app.wifiConnected;

    doc["ip"] =
        WiFi.localIP().toString();


    String timestamp = getTimestamp(app);

    doc["timestamp"] = timestamp;

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
}

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
    Serial.print("MQTT Nachricht auf Topic: ");
    Serial.println(topic);

    if (strcmp(topic, "weather/history/request") != 0)
    {
        return;
    }

    Serial.println("History-Anfrage empfangen!");

    JsonDocument doc;

    DeserializationError error =
        deserializeJson(doc, payload, length);

    if (error)
    {
        Serial.print("JSON Fehler: ");
        Serial.println(error.c_str());
        return;
    }

    if(!doc["type"].is<const char*>())
    {
        Serial.println("JSON Fehler: 'type' ist kein String");
        return;
    }

    const char* type = doc["type"];


    // =========================
    // Request: Sync-Info
    // =========================

    if (strcmp(type, "sync_info") == 0)
    {
        Serial.println("Request: Sync-Info");

        String response = sd_getSyncInfo();
        
        Serial.print("Antwort: ");
        Serial.println(response);

        mqttClient.publish(
            "weather/history/data",
            response.c_str()
        );

        return;
    }

    // =========================
    // Request: History
    // =========================

    if (strcmp(type, "history") == 0)
    {
        Serial.println("Request:History");
        JsonArray intervals = doc["intervals"];

        if (intervals.isNull())
        {
            Serial.println("JSON Fehler: 'intervals' fehlt");
            return;
        }

        sd_readHistoryIntervals(intervals);
        
        return;
    }


    Serial.println("Unbekannter Request-Typ");
}