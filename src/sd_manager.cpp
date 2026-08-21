#include "sd_manager.h"
#include "led.h"
#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>

#define SD_CS 5

struct HistoryEntry
{
    String time;
    float temperature;
    float humidity;
    float pressure;
};

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

    lastSDWrite = millis();

    Serial.println("SD: Messwert gespeichert.");
}

String sd_readHistory(int count)
{
    constexpr int MAX_HISTORY = 128;

    if (count <= 0)
    {
        return "{\"error\":\"invalid count\"}";
    }

    if (count > MAX_HISTORY)
    {
        count = MAX_HISTORY;
    }

    File file = SD.open(
        "/weather.csv",
        FILE_READ
    );

    if (!file)
    {
        return "{\"error\":\"file open failed\"}";
    }


    HistoryEntry historyData[MAX_HISTORY];

    int historyCount = 0;


    while (file.available())
    {
        String line =
            file.readStringUntil('\n');

        line.trim();


        // Header überspringen
        if (line ==
            "Zeit,Temperatur,Feuchte,Druck,Millis")
        {
            continue;
        }


        // CSV-Struktur prüfen
        int comma1 = line.indexOf(',');
        int comma2 =
            line.indexOf(',', comma1 + 1);
        int comma3 =
            line.indexOf(',', comma2 + 1);
        int comma4 =
            line.indexOf(',', comma3 + 1);

        if (comma1 == -1 ||
            comma2 == -1 ||
            comma3 == -1 ||
            comma4 == -1)
        {
            continue;
        }


        String time =
            line.substring(0, comma1);


        // Ungültiger Zeitstempel
        if (time.startsWith("0.0.0"))
        {
            continue;
        }


        float temperature = line.substring(comma1 + 1, comma2).toFloat();

        float humidity = line.substring(comma2 + 1, comma3).toFloat();

        float pressure = line.substring(comma3 + 1, comma4).toFloat();


        HistoryEntry entry;

        entry.time = time;
        entry.temperature = temperature;
        entry.humidity = humidity;
        entry.pressure = pressure;


        // History aufbauen
        if (historyCount < MAX_HISTORY)
        {
            historyData[historyCount++] = entry;
        }
        else
        {
            for (int i = 1; i < MAX_HISTORY; i++)
            {
                historyData[i - 1] = historyData[i];
            }

            historyData[MAX_HISTORY - 1] = entry;
        }
    }

    file.close();


    // Angeforderten Bereich bestimmen
    int startIndex = historyCount - count;

    if (startIndex < 0)
    {
        startIndex = 0;
    }


    // =========================
    // JSON erzeugen
    // =========================

    JsonDocument doc;

    JsonArray history =
        doc["history"].to<JsonArray>();


    for (int i = startIndex; i < historyCount;i++)
    {
        JsonObject entry =
            history.add<JsonObject>();

        entry["time"] = historyData[i].time;

        entry["temperature"] = historyData[i].temperature;

        entry["humidity"] = historyData[i].humidity;

        entry["pressure"] = historyData[i].pressure;
    }


    String output;

    serializeJson(doc, output);

    return output;
}