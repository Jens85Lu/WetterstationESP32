#pragma once
#include <Arduino.h>
#include "app_data.h"
#include <string>

void time_init();
void time_update();

String getTimestamp(const AppData& app);

struct WeekInfo
{
    int year;
    int week;
};

WeekInfo getWeekInfo(const struct tm& timeinfo);

bool getTimeInfo(struct tm& timeinfo);

WeekInfo getNextWeek(const WeekInfo& currentWeek);
