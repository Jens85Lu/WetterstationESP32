#include "history.h"
#include "app_data.h"

constexpr int N = 6; // später: HISTORY_AVERAGE_COUNT

static float sumTemp = 0.0f;
static float sumHumidity = 0.0f;
static float sumPressure = 0.0f;

static int historyCounter = 0;

void history_init() {
    // Hier könnte man z.B. die Historien-Arrays mit einem ungültigen Wert initialisieren, um zu erkennen, ob sie schon gefüllt wurden.
}



void history_update() {
    sumTemp += app.temp;
    sumHumidity += app.humidity;
    sumPressure += app.pressure;
    historyCounter++;

    if (historyCounter >= N) {
        // Durchschnitt der letzten N Werte berechnen
        float avgTemp = sumTemp / historyCounter;
        float avgHumidity = sumHumidity / historyCounter;
        float avgPressure = sumPressure / historyCounter;
    
        app.historyIndex = (app.historyIndex + 1) % 128; // Ringpuffer-Index aktualisieren

        app.tempHistory[app.historyIndex] = avgTemp;

        app.humidityHistory[app.historyIndex] = avgHumidity;

        app.pressureHistory[app.historyIndex] = avgPressure;

        if(app.validSamples < 128) {
            app.validSamples++;
        }
    
        // Sums zurücksetzen
        sumTemp = 0.0f;
        sumHumidity = 0.0f;
        sumPressure = 0.0f;
        historyCounter = 0;
    }
    updateMinMax();
    determineWeatherTendency();
}

void updateMinMax() {
  // Min/Max Temperatur bestimmen

  for (int i = 0; i < app.validSamples; ++i) {
    if (app.tempHistory[i] < app.minTemp) {
        app.minTemp = app.tempHistory[i];
    };
  };
  for (int i = 0; i < app.validSamples; ++i) {
    if (app.tempHistory[i] > app.maxTemp) {
      app.maxTemp = app.tempHistory[i];
    };
  };
  // Min/Max Luftfeuchtigkeit bestimmen
  
  for (int i = 0; i < app.validSamples; ++i) {
    if (app.humidityHistory[i] < app.minHumidity) {
      app.minHumidity = app.humidityHistory[i];
    };
  };
  for (int i = 0; i < app.validSamples; ++i) {
    if (app.humidityHistory[i] > app.maxHumidity) {
      app.maxHumidity = app.humidityHistory[i];
    };
  };
  // Min/Max Pressure bestimmen
  
  for (int i = 0; i < app.validSamples; ++i) {
    if (app.pressureHistory[i] < app.minPressure) {
      app.minPressure = app.pressureHistory[i];
    };
  };
  for (int i = 0; i < app.validSamples; ++i) {
    if (app.pressureHistory[i] > app.maxPressure) {
      app.maxPressure = app.pressureHistory[i];
    };
  };
};

void determineWeatherTendency() {
  // Wettertendenz bestimmen, wenn mindestens 30 gültige Messungen vorliegen
  if (app.validSamples >= 30) {
    int index30 = (app.historyIndex - 30 + 128) % 128; // Index für den Wert von vor 30 Messungen
    if (app.pressureHistory[app.historyIndex] > app.pressureHistory[index30] + 1.0f) {
      app.weatherTendency = 2; // Stark steigend
    } else if (app.pressureHistory[app.historyIndex] > app.pressureHistory[index30] + 0.5f) {
      app.weatherTendency = 1; // Steigend
    } else if (app.pressureHistory[app.historyIndex] < app.pressureHistory[index30] - 1.0f) {
      app.weatherTendency = -2; // Stark fallend
    } else if (app.pressureHistory[app.historyIndex] < app.pressureHistory[index30] - 0.5f) {
      app.weatherTendency = -1; // Stark fallend
    } else {
      app.weatherTendency = 0; // Stabil
    }
  }
  else {
    app.weatherTendency = 0; // Stabil, wenn nicht genügend Daten für Tendenzbestimmung vorliegen
  }
};