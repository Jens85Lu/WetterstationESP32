#include <Arduino.h>
#include "sensors.h"
#include "app_data.h"
#include "history.h"
#include "scheduler.h"
#include "display.h"
#include "button.h"


void setup() {
    delay(3000);
    Serial.begin(115200);
    delay(1000);
    sensors_init();    
    display_init();
    button_init();

}

void loop() {

    scheduler_run();
}