#include "weather_ui.h"
#include "app_data.h"

void weather_show(const AppData& app)
{
    display.clearBuffer();

    switch(app.currentScreen)
    {
        case SCREEN_MAIN:
            draw_main(app);
            break;
        case SCREEN_GRAPH_TEMP:
            draw_tempHistory(app);
            break;
        case SCREEN_GRAPH_HUM:
            draw_humHistory(app);
            break;
        case SCREEN_GRAPH_PRESSURE:
            draw_pressHistory(app);
            break;
        case SCREEN_SCREENSAVER:
            display.clearBuffer();
            break;
    }
    display.sendBuffer();
}


void drawWeatherTendency(int x, int y, int tendency)
{
    display.setFont(u8g2_font_open_iconic_weather_2x_t);

    switch (tendency)
    {
        case 2:
            display.drawGlyph(x, y, 69); // Sonne
            break;

        case 1:
            display.drawGlyph(x, y, 65); // Sonne/Wolke
            break;

        case 0:
            display.drawGlyph(x, y, 64); // Wolke
            break;

        case -1:
            display.drawGlyph(x, y, 67); // Regen
            break;

        case -2:
            display.drawGlyph(x, y, 66); // Gewitter/Regen
            break;
    }
}

void drawWifiIcon(int x, int y, bool connected)
{
    if (!connected)
    {
        display.drawLine(x , y + 4, x , y - 4);
        return;
    }

    // kleiner Balken
    display.drawBox(x - 8, y - 2, 2, 2);

    // mittlerer Balken
    display.drawBox(x - 4, y - 4, 2, 4);

    // großer Balken
    display.drawBox(x,     y - 6, 2, 6);
}

void drawDegreeSymbol(int x, int y)
{
    display.drawCircle(x, y, 2);
    display.drawCircle(x, y, 3);
}

void draw_main(const AppData& app)
{
    char buf[32];

    // Datum
    //display.setFont(u8g2_font_5x8_tr);
    display.setFont(u8g2_font_ncenB08_tr);
    snprintf(
        buf,
        sizeof(buf),
        "%02d.%02d.%04d",
        app.day,
        app.month,
        app.year
    );
    display.drawStr(0, 8, buf);

    // Uhrzeit
    snprintf(
        buf,
        sizeof(buf),
        "%02d:%02d",
        app.hour,
        app.minute
    );
    display.drawStr(70, 8, buf);

    //WiFi Symbol

    drawWifiIcon(118, 8, app.wifiConnected);

    // Wettertendenz
    drawWeatherTendency(110, 32, app.weatherTendency);

    // Temperatur groß
    display.setFont(u8g2_font_logisoso20_tr);

    snprintf(
        buf,
        sizeof(buf),
        "%.1f",
        app.temp
    );

    display.drawStr(20, 40, buf);
    display.drawStr(85, 40, "C");
    drawDegreeSymbol(80, 22);

    // Untere Zeile
    display.setFont(u8g2_font_helvB10_tr);
    // Feuchte
    snprintf(
        buf,
        sizeof(buf),
        "%.1f%%",
        app.humidity
    );
    display.drawStr(0, 62, buf);

    // Druck
    snprintf(
        buf,
        sizeof(buf),
        "%.1f hPa",
        app.pressure
    );
    display.drawStr(53, 62, buf);
}


void draw_tempHistory(const AppData& app) {

  float graphMin = floor(app.minTemp);
  float graphMax = ceil(app.maxTemp);
  int vRange = 63 - 14; // Vertikaler Bereich für den Graphen

  display.setFont(u8g2_font_ncenB08_tr);

    // Mindestrange von 1 Grad sicherstellen
    if (graphMin == graphMax) {
      graphMin = graphMin - 0.5f;
      graphMax = graphMax + 0.5f;
    }
    // Es werden zwei Datenpunkte gebraucht, um eine Linie zu zeichnen
    if (app.validSamples < 2) {
      display.drawStr(10, 30, "Collecting data...");
      return;
    }
    // Historie als Graph zeichnen, neueste Daten rechts
    for (int i = 1; i < app.validSamples; i++) {
      int index = (app.historyIndex - (i-1) + app.validSamples) % app.validSamples; // Ringpuffer-Index
      int prevIndex = (app.historyIndex - i + app.validSamples) % app.validSamples; // Vorheriger Index im Ringpuffer
      
      int x1 = 127 - i;
      int x2 = 127 - (i-1);
      
      int y1 = 63 - vRange*((app.tempHistory[prevIndex] - graphMin) / (graphMax - graphMin));
      int y2 = 63 - vRange*((app.tempHistory[index] - graphMin) / (graphMax - graphMin));

      display.drawLine(x1,y1,x2,y2);      
    }
    // Achsen zeichnen und Beschriftung hinzufügen
    display.drawLine(0, 14, 0, 63); // Y-Achse
    display.drawLine(0, 63, 127, 63); // X-Achse
    // Draw ticks on x-axis every 10 pixels
    for (int x = 10; x < 128; x += 10) {
      display.drawPixel(x, 62); // Ticks auf der X-Achse, 10 pixel à 6 minutes = 60 minutes
    }
    // Draw ticks on y-axis for every half integer temperatures
    for (int i = 0; i <= floor((graphMax-graphMin)*2); i++) {
      float t = graphMin + (float)i / 2.0f;
      int y = 63 - vRange*((t - graphMin) / (graphMax - graphMin));
      display.drawPixel(1, y); // Ticks auf der Y-Achse      
    }
    
    String header = "T: " + String(graphMin, 1) + " - " + String(graphMax, 1)  + "\xB0""C";
    display.drawStr(10, 12, header.c_str());
    String currentTempStr = String(app.temp, 1) + "\xB0""C";
    display.drawStr(100, 12, currentTempStr.c_str());

}

void draw_humHistory(const AppData& app) {

    float graphMin = floor(app.minHumidity);
    float graphMax = ceil(app.maxHumidity);
    int vRange = 63 - 14; // Vertikaler Bereich für den Graphen

    display.setFont(u8g2_font_ncenB08_tr);

    // Mindestrange von 1 % sicherstellen
    if (graphMin == graphMax) {
      graphMin = graphMin - 0.5f;
      graphMax = graphMax + 0.5f;
    }
    // Es werden zwei Datenpunkte gebraucht, um eine Linie zu zeichnen
    if (app.validSamples < 2) {
      display.drawStr(10, 30, "Collecting data...");
      return;
    }
    // Historie als Graph zeichnen, neueste Daten rechts
    for (int i = 1; i < app.validSamples; i++) {
      int index = (app.historyIndex - (i-1) + app.validSamples) % app.validSamples; // Ringpuffer-Index
      int prevIndex = (app.historyIndex - i + app.validSamples) % app.validSamples; // Vorheriger Index im Ringpuffer
      
      int x1 = 127 - i;
      int x2 = 127 - (i-1);
      
      int y1 = 63 - vRange*((app.humidityHistory[prevIndex] - graphMin) / (graphMax - graphMin));
      int y2 = 63 - vRange*((app.humidityHistory[index] - graphMin) / (graphMax - graphMin));

      display.drawLine(x1,y1,x2,y2);      
    }
    // Achsen zeichnen und Beschriftung hinzufügen
    display.drawLine(0, 14, 0, 63); // Y-Achse
    display.drawLine(0, 63, 127, 63); // X-Achse
    // Draw ticks on x-axis every 10 pixels
    for (int x = 10; x < 128; x += 10) {
      display.drawPixel(x, 62); // Ticks auf der X-Achse
    }
    // Draw ticks on y-axis for every integer humidity values
    for (int i = 0; i <= floor((graphMax-graphMin)); i++) {
      float t = graphMin + (float)i;
      int y = 63 - 49*((t - graphMin) / (graphMax - graphMin));
      display.drawPixel(1, y); // Ticks auf der Y-Achse      
    }
    
    String header = "H: " + String(graphMin, 1) + " - " + String(graphMax, 1)  + "%";
    display.drawStr(10, 12, header.c_str());
    String currentHumStr = String(app.humidity, 1) + "%";
    display.drawStr(100, 12, currentHumStr.c_str());
}


void draw_pressHistory(const AppData& app) {

  float graphMin = floor(app.minPressure);
  float graphMax = ceil(app.maxPressure);

  // Mindestrange von 1 hPa sicherstellen
    if (graphMin == graphMax) {
      graphMin = graphMin - 0.5f;
      graphMax = graphMax + 0.5f;
    }

  int vRange = 63 - 14; // Vertikaler Bereich für den Graphen

  display.setFont(u8g2_font_ncenB08_tr);

    // Es werden zwei Datenpunkte gebraucht, um eine Linie zu zeichnen
    if (app.validSamples < 2) {
      display.drawStr(10, 30, "Collecting data...");
      return;
    }
    // Historie als Graph zeichnen, neueste Daten rechts
    for (int i = 1; i < app.validSamples; i++) {
      int index = (app.historyIndex - (i-1) + app.validSamples) % app.validSamples; // Ringpuffer-Index
      int prevIndex = (app.historyIndex - i + app.validSamples) % app.validSamples; // Vorheriger Index im Ringpuffer
      
      int x1 = 127 - i;
      int x2 = 127 - (i-1);
      
      int y1 = 63 - vRange*((app.pressureHistory[prevIndex] - graphMin) / (graphMax - graphMin));
      int y2 = 63 - vRange*((app.pressureHistory[index] - graphMin) / (graphMax - graphMin));

      display.drawLine(x1,y1,x2,y2);      
    }
    // Achsen zeichnen und Beschriftung hinzufügen
    display.drawLine(0, 14, 0, 63); // Y-Achse
    display.drawLine(0, 63, 127, 63); // X-Achse

    // Draw ticks on x-axis every 10 pixels
    for (int x = 10; x < 128; x += 10) {
      display.drawPixel(x, 62); // Ticks auf der X-Achse
    }
    // Draw ticks on y-axis for every half of an integer value
    for (int i = 0; i <= floor((graphMax-graphMin)*2); i++) {
      float t = graphMin + (float)i / 2.0f;
      int y = 63 - 49*((t - graphMin) / (graphMax - graphMin));
      display.drawPixel(1, y); // Ticks auf der Y-Achse      
    }
    
    String header = "P: " + String(graphMin, 1) + " - " + String(graphMax, 1) + " hPa";
    display.drawStr(10, 12, header.c_str());
    //String currentPressStr = String(app.pressure_seaLevel, 1) + " hPa";
    //display.drawStr(100, 12, currentPressStr.c_str());
}
