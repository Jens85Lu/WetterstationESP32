#include "scheduler.h"
#include "sensors.h"
#include "app_data.h"
#include "history.h"
#include <Arduino.h>
#include "display.h"
#include "ui_manager.h"
#include "mqtt.h"

unsigned long now = 0;

unsigned long lastSensorTime = 0;
const unsigned long sensorInterval = 2000; // 2 Sekunden
unsigned long lastDisplayTime = 0;
const unsigned long displayInterval = 200; // 2 Sekunden

void scheduler_run()
{
    now = millis();

    // =========================
    // Sensoren
    // =========================

    if (now - lastSensorTime >= sensorInterval) {

        lastSensorTime = now;

        sensors_read();
        update_appData();
        history_update();
    }


    // =========================
    // Display
    // =========================

    if (now - lastDisplayTime >= displayInterval) {

        lastDisplayTime = now;

        display_update();
    }


    // =========================
    // UI
    // =========================

    ui_update();
}
