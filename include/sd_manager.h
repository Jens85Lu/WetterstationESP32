#pragma once

#include "app_data.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "time_manager.h"


void sd_init();


void sd_saveData(
    const AppData& app,
    float meanTemp,
    float meanHumidity,
    float meanPressure
);

bool sd_isReady();
bool sd_wasRecentlyWritten();

bool sd_weekFileHasData(const String& filename);

bool sd_getFirstWeek(WeekInfo& firstWeek);
bool sd_getLatestWeek(WeekInfo& latestWeek);

std::string sd_getWeekFilename(const WeekInfo& weekInfo);
String sd_getFirstTimestamp();
String sd_getLatestTimestamp();

bool parseTimestamp(const String& timestamp, struct tm& timeinfo);
bool getHistoryWeekRange(JsonArray& intervals, WeekInfo& firstWeek, WeekInfo& lastWeek);

String sd_getSyncInfo();

String sd_readHistoryIntervals(JsonArray& intervals);

String sd_readHistoryDebug(String from, String to);

