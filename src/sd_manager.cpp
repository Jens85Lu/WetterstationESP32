#include "sd_manager.h"
#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>
#include "time_manager.h"
#include "mqtt.h"

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


// ==================================
// Jahresverzeichnis sicherstellen
// ==================================

bool sd_ensureYearDirectory(int year)
{
    if (!SD.exists("/weather"))
    {
        if (!SD.mkdir("/weather"))
        {
            Serial.println(
                "FEHLER: /weather konnte nicht angelegt werden!"
            );

            return false;
        }
    }

    String yearPath = "/weather/" + String(year);

    if (!SD.exists(yearPath))
    {
        if (!SD.mkdir(yearPath))
        {
            Serial.print(
                "FEHLER: Jahresverzeichnis konnte nicht angelegt werden: "
            );
            Serial.println(yearPath);

            return false;
        }
    }

    return true;
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

    if (!app.timeValid)
    {
        Serial.println(
            "Weltzeit nicht gueltig - Speichern uebersprungen."
        );

        return;
    }

    // Aktuelle Zeit holen

    struct tm timeinfo;

    if (!getTimeInfo(timeinfo))
    {
        Serial.println(
            "Zeitinfo konnte nicht gelesen werden - "
            "Speichern uebersprungen."
        );

        return;
    }

    // ISO-Jahr und ISO-Woche bestimmen

    WeekInfo weekInfo = getWeekInfo(timeinfo);

    // Jahresverzeichnis sicherstellen

    if (!sd_ensureYearDirectory(weekInfo.year))
    {
        return;
    }

    // Dateiname bestimmen

    std::string filename = sd_getWeekFilename(weekInfo);

    bool fileExists = SD.exists(filename.c_str());

    // Wochen-Datei öffnen

    File file = SD.open(
        filename.c_str(),
        FILE_APPEND
    );

    if (!file)
    {
        Serial.print(
            "FEHLER: Wochen-Datei konnte nicht geoeffnet werden: "
        );
        Serial.println(filename.c_str());

        sdReady = false;
        return;
    }

    // Header nur bei neuer Datei schreiben

    if (!fileExists)
    {
        file.println(
            "Zeit,Temperatur,Feuchte,Druck,Millis"
        );

        Serial.print("SD: Neue Wochen-Datei angelegt: ");
        Serial.println(filename.c_str());
    }

    // Zeitstempel

    file.print(getTimestamp(app));

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
    Serial.print("T= ");
    Serial.print(meanTemp);
    Serial.print(", ");
    Serial.print("H= ");
    Serial.print(meanHumidity);
    Serial.print(", ");
    Serial.print("P= ");
    Serial.println(meanPressure);

}

String sd_getFirstTimestamp()
{
    if (!sdReady)
    {
        return "";
    }

    WeekInfo firstWeek;

    if (!sd_getFirstWeek(firstWeek))
    {
        Serial.println(
            "FEHLER: Erste Woche konnte nicht bestimmt werden."
        );

        return "";
    }

    String filename =
        "/weather/" +
        String(firstWeek.year) +
        "/weather_" +
        String(firstWeek.year) +
        "-W" +
        String(firstWeek.week) +
        ".csv";

    File file = SD.open(
        filename,
        FILE_READ
    );

    if (!file)
    {
        Serial.print(
            "FEHLER: Wochen-Datei konnte nicht gelesen werden: "
        );
        Serial.println(filename);

        return "";
    }

    // Header überspringen
    if (!file.available())
    {
        file.close();
        return "";
    }

    file.readStringUntil('\n');

    // Erste Datenzeile suchen
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();

        if (line.length() == 0)
        {
            continue;
        }

        int comma = line.indexOf(',');

        if (comma == -1)
        {
            continue;
        }

        String firstTimestamp =
            line.substring(0, comma);

        if (firstTimestamp.length() == 0)
        {
            continue;
        }

        file.close();

        return firstTimestamp;
    }

    file.close();

    return "";
}

String sd_getLatestTimestamp()
{
    if (!sdReady)
    {
        return "";
    }

    WeekInfo latestWeek;

    if (!sd_getLatestWeek(latestWeek))
    {
        Serial.println(
            "FEHLER: Could not retrieve latest week."
        );

        return "";
    }

    String filename =
        "/weather/" +
        String(latestWeek.year) +
        "/weather_" +
        String(latestWeek.year) +
        "-W" +
        String(latestWeek.week) +
        ".csv";

    File file = SD.open(
        filename,
        FILE_READ
    );

    if (!file)
    {
        Serial.println("FEHLER: Wochendatei konnte nicht gelesen werden:");
        Serial.println(filename);
        return "";
    }

    String latestTimestamp = "";

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();

        // Header überspringen
        if (line == "Zeit,Temperatur,Feuchte,Druck,Millis")
        {
            continue;
        }

        int comma = line.indexOf(',');

        if (comma == -1)
        {
            continue;
        }

        String timestamp = line.substring(0, comma);

        // Ungültige Zeile überspringen
        if (timestamp.length() == 0)
        {
            continue;
        }

        latestTimestamp = timestamp;
    }

    file.close();

    return latestTimestamp;
}

bool sd_weekFileHasData(const String& filename)
{
    File weekFile = SD.open(filename, FILE_READ);

    if (!weekFile)
    {
        return false;
    }

    // Header überspringen
    if (!weekFile.available())
    {
        weekFile.close();
        return false;
    }

    weekFile.readStringUntil('\n');

    // Gibt es danach eine Datenzeile?
    while (weekFile.available())
    {
        String line = weekFile.readStringUntil('\n');
        line.trim();

        if (line.length() > 0)
        {
            weekFile.close();
            return true;
        }
    }

    weekFile.close();
    return false;
}

bool sd_getFirstWeek(WeekInfo& firstWeek)
{
    if (!sdReady)
    {
        return false;
    }

    File root = SD.open("/weather");

    if (!root || !root.isDirectory())
    {
        Serial.println(
            "FEHLER: /weather konnte nicht geoeffnet werden."
        );

        return false;
    }

    int firstYear = 0;
    int firstWeekNumber = 0;

    File entry = root.openNextFile();

    while (entry)
    {
        if (entry.isDirectory())
        {
            String yearName = entry.name();

            int year = yearName.toInt();

            String yearPath =
                "/weather/" + yearName;

            File yearDir = SD.open(yearPath);

            if (yearDir && yearDir.isDirectory())
            {
                File weekFile = yearDir.openNextFile();

                while (weekFile)
                {
                    if (!weekFile.isDirectory())
                    {
                        String filename =
                            weekFile.name();

                        String prefix =
                            "weather_" +
                            yearName +
                            "-W";

                        if (
                            filename.startsWith(prefix) &&
                            filename.endsWith(".csv")
                        )
                        {
                            String weekString =
                                filename.substring(
                                    prefix.length(),
                                    filename.length() - 4
                                );

                            int week =
                                weekString.toInt();

                            if (week >= 1 && week <= 53)
                            {
                                String fullPath =
                                    yearPath +
                                    "/" +
                                    filename;

                                if (sd_weekFileHasData(fullPath))
                                {
                                    if (
                                        firstYear == 0 ||
                                        year < firstYear ||
                                        (
                                            year == firstYear &&
                                            week < firstWeekNumber
                                        )
                                    )
                                    {
                                        firstYear = year;
                                        firstWeekNumber = week;
                                    }
                                }
                            }
                        }
                    }

                    weekFile.close();
                    weekFile = yearDir.openNextFile();
                }

                yearDir.close();
            }
        }

        entry.close();
        entry = root.openNextFile();
    }

    root.close();

    if (firstYear == 0)
    {
        return false;
    }

    firstWeek.year = firstYear;
    firstWeek.week = firstWeekNumber;

    return true;
}


bool sd_getLatestWeek(WeekInfo& latestWeek)
{
    if (!sdReady)
    {
        return false;
    }

    File root = SD.open("/weather");

    if (!root || !root.isDirectory())
    {
        Serial.println(
            "FEHLER: /weather konnte nicht geoeffnet werden."
        );

        return false;
    }

    int latestYear = 0;
    int latestWeekNumber = 0;

    File entry = root.openNextFile();

    while (entry)
    {
        if (entry.isDirectory())
        {
            String yearName = entry.name();

            int year = yearName.toInt();

            String yearPath =
                "/weather/" + yearName;

            File yearDir = SD.open(yearPath);

            if (yearDir && yearDir.isDirectory())
            {
                File weekFile = yearDir.openNextFile();

                while (weekFile)
                {
                    if (!weekFile.isDirectory())
                    {
                        String filename =
                            weekFile.name();

                        String prefix =
                            "weather_" +
                            yearName +
                            "-W";

                        if (
                            filename.startsWith(prefix) &&
                            filename.endsWith(".csv")
                        )
                        {
                            String weekString =
                                filename.substring(
                                    prefix.length(),
                                    filename.length() - 4
                                );

                            int week =
                                weekString.toInt();

                            if (week >= 1 && week <= 53)
                            {
                                String fullPath =
                                    yearPath +
                                    "/" +
                                    filename;

                                if (sd_weekFileHasData(fullPath))
                                {
                                    if (
                                        year > latestYear ||
                                        (
                                            year == latestYear &&
                                            week > latestWeekNumber
                                        )
                                    )
                                    {
                                        latestYear = year;
                                        latestWeekNumber = week;
                                    }
                                }
                            }
                        }
                    }

                    weekFile.close();
                    weekFile = yearDir.openNextFile();
                }

                yearDir.close();
            }
        }

        entry.close();
        entry = root.openNextFile();
    }

    root.close();

    if (latestYear == 0)
    {
        return false;
    }

    latestWeek.year = latestYear;
    latestWeek.week = latestWeekNumber;

    return true;
}

String sd_getSyncInfo()
{
    if (!sdReady)
        return "{\"error\":\"sd not ready\"}";

   String firstTimestamp = sd_getFirstTimestamp();
   String lastTimestamp = sd_getLatestTimestamp();

   if (
        firstTimestamp.length() == 0 ||
        lastTimestamp.length() == 0
    )
    {
        return "{\"error\":\"no weather data found\"}";
    }

    JsonDocument doc;

    doc["type"] = "sync_info";
    doc["first"] = firstTimestamp;
    doc["last"] = lastTimestamp;

    String output;

    serializeJson(doc, output);

    return output;
}

String sd_readHistoryIntervals(JsonArray& intervals)
{
    WeekInfo firstWeek;
    WeekInfo lastWeek;

    if (!getHistoryWeekRange(intervals, firstWeek, lastWeek))
    {
        return "{\"error\":\"invalid history intervals\"}";
    }

    WeekInfo currentWeek = firstWeek;

    JsonDocument doc;
    doc["type"] = "history";
    JsonArray history = doc["history"].to<JsonArray>();

    int count = 0;
    int chunk = 0;

    Serial.println("Empfangene Intervalle:");

    for (JsonObject interval : intervals)
    {
        const char* from = interval["from"];
        const char* to = interval["to"];

        Serial.print("  from: ");
        Serial.println(from);

        Serial.print("  to:   ");
        Serial.println(to);
    }

    while (true)
    {
        std::string filename =
            sd_getWeekFilename(currentWeek);

        Serial.print("Lese Wochen-Datei: ");
        Serial.println(filename.c_str());

        File file = SD.open(
            filename.c_str(),
            FILE_READ
        );

        if (file)
        {
            while(file.available())
            {
                String line = file.readStringUntil('\n');
                line.trim();

                // Header überspringen
                if(line == "Zeit,Temperatur,Feuchte,Druck,Millis")
                    continue;

                int comma1 = line.indexOf(',');
                int comma2 = line.indexOf(',', comma1 + 1);
                int comma3 = line.indexOf(',', comma2 + 1);
                int comma4 = line.indexOf(',', comma3 + 1);

                if(comma1 == -1 || comma2 == -1 || comma3 == -1 || comma4 == -1)
                    continue;

                String timestamp = line.substring(0, comma1);

                // Leere Zeilen ignorieren
                if(timestamp.startsWith("0"))
                    continue;

                bool requested = false;

                for (JsonObject interval : intervals)
                {
                    const char* from =
                        interval["from"];

                    const char* to =
                        interval["to"];

                    String fromFull =
                        String(from) + ":00";

                    String toFull =
                        String(to) + ":59";

                    if (timestamp >= fromFull &&
                        timestamp <= toFull)
                    {
                        Serial.print("Treffer: ");
                        Serial.println(timestamp);
                        requested = true;
                        break;
                    }
                }

                if(!requested) // if data point requested, i.e. with a given interval
                    continue;

                float temperature =
                    line.substring(
                        comma1 + 1,
                        comma2
                    ).toFloat();

                float humidity =
                    line.substring(
                        comma2 + 1,
                        comma3
                    ).toFloat();

                float pressure =
                    line.substring(
                        comma3 + 1,
                        comma4
                    ).toFloat();

                JsonObject entry =
                    history.add<JsonObject>();
                
                entry["timestamp"] = timestamp;
                entry["temp"] = temperature;
                entry["humidity"] = humidity;
                entry["pressure"] = pressure;

                
                count++;
                if(count == 10)
                {
                    chunk++;
                    Serial.print("Zwischenspeicher: ");
                    Serial.println(chunk);
                    doc["chunk"] = chunk;
                    doc["last"] = false;

                    String output;
                    serializeJson(doc, output);

                    mqttClient.publish(
                        "weather/history/data",
                        output.c_str()
                    );
                    history.clear();
                    count = 0;
                }
            }

            
            file.close();
        }
        else
        {
            Serial.print("Wochen-Datei konnte nicht geoeffnet werden: ");
            Serial.println(filename.c_str());
        }

        // Letzte benötigte Woche erreicht?
        if (
            currentWeek.year == lastWeek.year &&
            currentWeek.week == lastWeek.week
        )
        {
            break;
        }

        currentWeek = getNextWeek(currentWeek);
    }

    // Restliche Daten senden, falls vorhanden
    if (count > 0)
    {
        chunk++;
        Serial.print("Letzter Chunk: ");
        Serial.println(chunk);
        doc["last"] = true;
        doc["chunk"] = chunk;

        String output;
        serializeJson(doc, output);

        mqttClient.publish(
            "weather/history/data",
            output.c_str()
        );
    }
    else
    {
        // Der letzte Chunk war exakt voll.
        // Deshalb noch eine Abschlussnachricht senden.

        chunk++;

        doc["chunk"] = chunk;
        doc["last"] = true;

        String output;
        serializeJson(doc, output);

        mqttClient.publish(
            "weather/history/data",
            output.c_str()
        );
    }

    return "";
}

std::string sd_getWeekFilename(const WeekInfo& weekInfo)
{

    char filename[64];

    snprintf(filename,
             sizeof(filename),
             "/weather/%04d/weather_%04d-W%02d.csv",
             weekInfo.year,
             weekInfo.year,
             weekInfo.week);

    return std::string(filename);
}

bool parseTimestamp(
    const String& timestamp,
    struct tm& timeinfo
)
{
    memset(&timeinfo, 0, sizeof(timeinfo));

    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second = 0;

    int result = sscanf(
        timestamp.c_str(),
        "%d-%d-%d %d:%d:%d",
        &year,
        &month,
        &day,
        &hour,
        &minute,
        &second
    );

    if (result < 5)
    {
        return false;
    }

    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;

    mktime(&timeinfo);

    return true;
}

bool getHistoryWeekRange(
    JsonArray& intervals,
    WeekInfo& firstWeek,
    WeekInfo& lastWeek
)
{
    if (intervals.isNull() || intervals.size() == 0)
    {
        return false;
    }

    JsonObject firstInterval = intervals[0];
    JsonObject lastInterval = intervals[intervals.size() - 1];

    const char* firstTimestamp = firstInterval["from"];
    const char* lastTimestamp = lastInterval["to"];

    if (firstTimestamp == nullptr ||
        lastTimestamp == nullptr)
    {
        return false;
    }

    struct tm firstTime;
    struct tm lastTime;

    if (!parseTimestamp(
            String(firstTimestamp),
            firstTime))
    {
        return false;
    }

    if (!parseTimestamp(
            String(lastTimestamp),
            lastTime))
    {
        return false;
    }

    firstWeek = getWeekInfo(firstTime);
    lastWeek = getWeekInfo(lastTime);

    return true;
}