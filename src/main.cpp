#include <Arduino.h>
#include "sensors.h"
#include "app_data.h"
#include "history.h"
#include "scheduler.h"
#include "display.h"
#include "button.h"
#include "wifi_manager.h"
#include <time.h>
#include "time_manager.h"
#include "wifi_manager.h"


void setup() {
    delay(3000);
    Serial.begin(115200);
    delay(1000);
    sensors_init();    
    display_init();
    button_init();
    wifi_init();
    time_init();
}


void loop() {
    
    wifi_update();

    scheduler_run();

    static unsigned long lastTimeUpdate = 0;
    
    if (millis() - lastTimeUpdate > 1000)
    {
        lastTimeUpdate = millis();
        time_update();
    }
}