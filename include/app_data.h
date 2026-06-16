#pragma once

enum uiScreen {
  SCREEN_MAIN,
  SCREEN_MINMAX,
  SCREEN_GRAPH_TEMP,
  SCREEN_GRAPH_HUM,
  SCREEN_GRAPH_PRESSURE,
  SCREEN_TREND,
  SCREEN_SCREENSAVER
};

struct AppData
{
    // Sensordaten
    float temp;
    float humidity;
    float pressure;
    float bmpTemperature;
    float pressure_seaLevel;

    float minTemp = 999.0f;
    float maxTemp = -999.0f ;
    float minHumidity = 999.0f;
    float maxHumidity = -999.0f;
    float minPressure = 9999.0f;
    float maxPressure = -9999.0f;

    // Historie
    float tempHistory[128];
    float humidityHistory[128];
    float pressureHistory[128];
    int weatherTendency; // -2 = stark fallend, -1 = fallend, 0 = stabil, 1 = steigend, 2 = stark steigend
    int historyIndex = -1;
    int validSamples;

    /* // Mittelwerte für history vielleicht später mal nötig
    float meanTemp;
    float meanHumidity;
    float meanPressure;
    */

    bool sensorValid = false;   
    bool ledState; 

    // UI
    uiScreen currentScreen = SCREEN_MAIN;
    unsigned long lastButtonTime = 0;
};

void update_appData();

extern AppData app;




