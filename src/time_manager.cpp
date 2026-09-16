#include "time_manager.h"
#include "app_data.h"
#include <WiFi.h>
#include <Arduino.h>
#include <time.h>

void time_init() {

    configTime(
        3600, //UTC + 1
        3600, // Sommerzeit
        "pool.ntp.org");

        /* später:
        setenv("TZ",
       "CET-1CEST,M3.5.0,M10.5.0/3",
       1);
        tzset();*/
}

bool getTimeInfo(struct tm& timeinfo)
{
    
    if (!getLocalTime(&timeinfo, 10))
    {
        Serial.println("Zeitinfo holen nicht erfolgreich.");
        return false;
    }

    return true;
}

void time_update() {
    
    if (!app.timeValid)
        return;

    struct tm timeinfo;

    if (!getTimeInfo(timeinfo))
        return;

    app.hour = timeinfo.tm_hour;
    app.minute = timeinfo.tm_min;
    app.second = timeinfo.tm_sec;
    
    app.day = timeinfo.tm_mday;
    app.month = timeinfo.tm_mon + 1;
    app.year = timeinfo.tm_year + 1900;
}

String getTimestamp(const AppData& app)
{
    char timestamp[20];

    snprintf(
        timestamp,
        sizeof(timestamp),
        "%04d-%02d-%02d %02d:%02d:%02d",
        app.year,
        app.month,
        app.day,
        app.hour,
        app.minute,
        app.second
    );
    
    return String(timestamp);
}

WeekInfo getWeekInfo(const struct tm& timeinfo)
{
    WeekInfo info;

    char year[5];
    char week[3];

    strftime(year, sizeof(year), "%G", &timeinfo);
    strftime(week, sizeof(week), "%V", &timeinfo);

    info.year = atoi(year);
    info.week = atoi(week);

    return info;

}


WeekInfo getNextWeek(const WeekInfo& currentWeek)
{
    struct tm timeinfo = {};

    timeinfo.tm_year = currentWeek.year - 1900;
    timeinfo.tm_mon  = 0;
    timeinfo.tm_mday = 4;

    mktime(&timeinfo);

    timeinfo.tm_mday += currentWeek.week * 7;

    mktime(&timeinfo);

    return getWeekInfo(timeinfo);
}
