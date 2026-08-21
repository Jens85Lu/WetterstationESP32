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
}

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
    Serial.print("MQTT Nachricht auf Topic: ");
    Serial.println(topic);

    if (strcmp(topic, "weather/history/request") == 0)
    {
        Serial.println("History-Anfrage empfangen!");

        JsonDocument doc;

        DeserializationError error = deserializeJson(doc, payload, length);

        if (error)
        {
            Serial.print("JSON Fehler: ");
            Serial.println(error.c_str());
            return;
        }

        int count = doc["count"] | 0;

        Serial.print("Anzahl angefordert: ");
        Serial.println(count);

        String response = sd_readHistory(count);

        Serial.print("MQTT connected: ");
        Serial.println(mqttClient.connected());

        Serial.print("Payload length: ");
        Serial.println(response.length());

        Serial.print("MQTT buffer size: ");
        Serial.println(mqttClient.getBufferSize());

        bool success = mqttClient.publish(
            "weather/history/data",
            response.c_str()
        );

        if (success)
        {
            Serial.println("MQTT History publish erfolgreich.");
        }
        else
        {
            Serial.println("MQTT History publish FEHLGESCHLAGEN!");
        }
    }
}