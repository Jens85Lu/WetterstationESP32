#include "scheduler.h"
#include "sensors.h"
#include "led.h"
#include "app_data.h"
#include "averaging.h"
#include <Arduino.h>
#include "display.h"
#include "ui_manager.h"
#include "mqtt.h"
#include "sd_manager.h"

unsigned long now = 0;

unsigned long lastSensorTime = 0;
const unsigned long sensorInterval = 2000; // 2 Sekunden
unsigned long lastDisplayTime = 0;
const unsigned long displayInterval = 200; // 2 Sekunden

void scheduler_run()
{
    now = millis();

    // =========================
    // Sensoren lesen, Mittelwerte aktualisieren, Daten speichern und senden
    // =========================

    if (now - lastSensorTime >= sensorInterval) {

        lastSensorTime = now;

        sensors_read();
        update_appData();
        oled_history_update();
        
        if (mean_update()) {

            sd_saveData(app, app.tempMean, app.humidityMean, app.pressureMean);

            sendLiveData(app);
        }
    }


    // =========================
    // SD-Karte Schreibanzeige und Initialisierung
    // =========================
    
    static unsigned long lastSDCheck = 0;

    if(!sd_isReady() && millis() - lastSDCheck >= 30000) {
        lastSDCheck = millis();
        sd_init();
    }

    if (sd_wasRecentlyWritten())
    {
        led_on();
    }
    else
    {
        led_off();
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
