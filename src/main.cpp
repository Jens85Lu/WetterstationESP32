#include <Arduino.h>
#include <ArduinoOTA.h>

#include "sensors.h"
#include "app_data.h"
#include "averaging.h"
#include "scheduler.h"
#include "display.h"
#include "button.h"
#include "led.h"
#include "wifi_manager.h"
#include "time_manager.h"
#include "mqtt.h"
#include "sd_manager.h"
#include <WebServer.h>
#include <SD.h>

WebServer server(80);

void setup() {

    delay(3000);

    Serial.begin(115200);
    delay(1000);
    
    sensors_init();
    display_init();
    button_init();
    led_init();
    sd_init();
    wifi_init();
    Serial.println(WiFi.localIP());

    // =========================
    // OTA (over the air flashing)
    // =========================

    ArduinoOTA.setHostname("WetterstationESP32");

    ArduinoOTA.onStart([]() {
        Serial.println("OTA Start");
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\nOTA Ende");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf(
            "OTA: %u%%\r",
            (progress * 100) / total
        );
    });

    ArduinoOTA.onError([](ota_error_t error) {

        Serial.printf(
            "OTA Fehler [%u]\n",
            error
        );

    });

    ArduinoOTA.begin();

    Serial.println("OTA bereit!");


    // =========================
    // Zeit / MQTT
    // =========================

    time_init();
    mqtt_init();

    // =========================
    // Webserver
    // =========================

    server.on("/api/history", HTTP_GET, []() {

        if (!server.hasArg("from") || !server.hasArg("to"))
        {
            server.send(
                400,
                "application/json",
                "{\"error\":\"Parameter from und to erforderlich\"}"
            );
            return;
        }

        String from = server.arg("from");
        String to = server.arg("to");

        String response = sd_readHistoryDebug(from, to);

        server.send(
            200,
            "application/json",
            response
        );
    });

    server.begin();

    Serial.println("Webserver bereit!");
}


void loop() {

    wifi_update();

    // OTA muss regelmäßig aufgerufen werden
    ArduinoOTA.handle();


    // =========================
    // Scheduler
    // =========================

    scheduler_run();


    // =========================
    // Zeit
    // =========================

    static unsigned long lastTimeUpdate = 0;

    if (millis() - lastTimeUpdate > 1000)
    {
        lastTimeUpdate = millis();
        time_update();
    }


    // =========================
    // MQTT
    // =========================

    static unsigned long lastMQTTTry = 0;

    if (
        app.wifiConnected &&
        !mqttClient.connected() &&
        millis() - lastMQTTTry >= 60000
    )
    {
        lastMQTTTry = millis();
        mqtt_connect();
    }
    
    mqttClient.loop();

    // =========================
    // Webserver
    // =========================
    
    server.handleClient();
}