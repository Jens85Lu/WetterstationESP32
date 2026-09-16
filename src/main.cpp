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

#include <SD.h>

void testHistoryWeekRange()
{
    JsonDocument doc;

    JsonArray intervals = doc.to<JsonArray>();

    JsonObject interval1 = intervals.add<JsonObject>();
    interval1["from"] = "2026-12-31 23:58";
    interval1["to"]   = "2027-01-01 00:02";

    JsonObject interval2 = intervals.add<JsonObject>();
    interval2["from"] = "2027-01-03 12:00";
    interval2["to"]   = "2027-01-04 00:30";

    WeekInfo firstWeek;
    WeekInfo lastWeek;

    bool success = getHistoryWeekRange(
        intervals,
        firstWeek,
        lastWeek
    );

    if (!success)
    {
        Serial.println("getHistoryWeekRange fehlgeschlagen.");
        return;
    }

    Serial.println("getHistoryWeekRange erfolgreich.");

    Serial.print("First Week: ");
    Serial.print(firstWeek.year);
    Serial.print("-W");
    Serial.println(firstWeek.week);

    Serial.print("Last Week: ");
    Serial.print(lastWeek.year);
    Serial.print("-W");
    Serial.println(lastWeek.week);
}


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
    testHistoryWeekRange();


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

    if (millis() - lastTimeUpdate > 10000)
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
        millis() - lastMQTTTry >= 10000
    )
    {
        lastMQTTTry = millis();
        mqtt_connect();
    }
    
    mqttClient.loop();

}