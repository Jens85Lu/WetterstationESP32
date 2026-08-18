#include "sd_manager.h"

#include <Arduino.h>
#include <SD.h>

#define SD_CS 5

static bool sdReady = false;

static unsigned long lastSDWrite = 0;

bool sd_wasRecentlyWritten()
{
    return (millis() - lastSDWrite) < 2000;
}


// =========================
// SD initialisieren
// =========================

void sd_init()
{
    Serial.println("SD Initialisierung...");

    for (int i = 0; i < 3; i++)
    {
        Serial.print("Versuch ");
        Serial.println(i + 1);

        if (SD.begin(SD_CS))
        {
            sdReady = true;

            Serial.println("SD Karte bereit.");

            File file = SD.open(
            "/weather.csv",
            FILE_APPEND);
            file.print("Zeit,Temperatur,Feuchte,Druck,Millis\n");
            file.close();
            Serial.println("SD: Header gespeichert.");

            return;
        }

        Serial.println("SD Initialisierung fehlgeschlagen.");

        delay(500);
    }

    sdReady = false;

    Serial.println(
        "FEHLER: SD-Karte konnte nicht initialisiert werden!"
    );
}


// =========================
// SD Status
// =========================

bool sd_isReady()
{
    return sdReady;
}


// =========================
// Daten speichern
// =========================

void sd_saveData(
    const AppData& app,
    float meanTemp,
    float meanHumidity,
    float meanPressure
)
{
    if (!sdReady)
    {
        Serial.println(
            "SD nicht bereit - Speichern uebersprungen."
        );

        return;
    }

    File file = SD.open(
        "/weather.csv",
        FILE_APPEND
    );

    if (!file)
    {
        Serial.println(
            "FEHLER: weather.csv konnte nicht geoeffnet werden!"
        );

        sdReady = false;
        return;
    }

    // Datum
    file.print(app.day);
    file.print(".");
    file.print(app.month);
    file.print(".");
    file.print(app.year);

    file.print(" ");

    // Uhrzeit
    if (app.hour < 10)
        file.print("0");

    file.print(app.hour);

    file.print(":");

    if (app.minute < 10)
        file.print("0");

    file.print(app.minute);

    // Werte
    file.print(",");
    file.print(meanTemp, 2);

    file.print(",");
    file.print(meanHumidity, 2);

    file.print(",");
    file.print(meanPressure, 2);

    file.print(",");
    file.println(millis());

    file.close();

    Serial.println(
        "SD: Messwert gespeichert."
    );
    lastSDWrite = millis();
}