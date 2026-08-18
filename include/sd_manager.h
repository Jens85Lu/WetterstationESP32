#pragma once

#include "app_data.h"

void sd_init();

void sd_saveData(
    const AppData& app,
    float meanTemp,
    float meanHumidity,
    float meanPressure
);

bool sd_isReady();
bool sd_wasRecentlyWritten();