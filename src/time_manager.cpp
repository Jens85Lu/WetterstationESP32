#include "time_manager.h"
#include "app_data.h"
#include <WiFi.h>

#include <time.h>

void time_init() {

    configTime(
        3600, //UTC + 1
        3600, // Sommerzeig
        "pool.ntp.org");

        /* später:
        setenv("TZ",
       "CET-1CEST,M3.5.0,M10.5.0/3",
       1);
        tzset();*/
}

void time_update() {
    
    struct tm timeinfo;

        if (!getLocalTime(&timeinfo)) {
        Serial.println("No time available");
        return;
    }

    app.hour = timeinfo.tm_hour;
    app.minute = timeinfo.tm_min;
    
    app.day = timeinfo.tm_mday;
    app.month = timeinfo.tm_mon + 1;
    app.year = timeinfo.tm_year + 1900;
}