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

        case SCREEN_MINMAX:
            draw_minmax(app);
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
        case SCREEN_TREND:
            draw_trend(app);
            break;
        case SCREEN_SCREENSAVER:
            display.clearBuffer();
            break;
    }
    display.sendBuffer();
}

void draw_main(const AppData& app) {

    char buf[32];

    display.setFont(u8g2_font_ncenB08_tr); //u8g2_font_6x12_tr

    snprintf(buf, sizeof(buf), "Temp: %.1f C", app.temp);
    display.drawStr(0, 15, buf);

    snprintf(buf, sizeof(buf), "Hum : %.1f %%", app.humidity);
    display.drawStr(0, 30, buf);

    snprintf(buf, sizeof(buf), "Press: %.1f hPa", app.pressure);
    display.drawStr(0, 45, buf);

    snprintf(
    buf,
    sizeof(buf),
    "%02d:%02d",
    app.hour,
    app.minute
    );
    display.drawStr(80, 15, buf);

    snprintf(
    buf,
    sizeof(buf),
    "%02d.%02d.%04d",
    app.day,
    app.month,
    app.year
    );
    display.drawStr(50, 63, buf);

}

  void draw_minmax(const AppData& app) {

    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);

    char buf[32];

    snprintf(buf, sizeof(buf), "Tmin: %.1f C", app.minTemp);
    display.drawStr(0, 12, buf);

    snprintf(buf, sizeof(buf), "Tmax: %.1f C", app.maxTemp);
    display.drawStr(0, 24, buf);

    snprintf(buf, sizeof(buf), "Hmin: %.1f %%", app.minHumidity);
    display.drawStr(0, 40, buf);

    snprintf(buf, sizeof(buf), "Hmax: %.1f %%", app.maxHumidity);
    display.drawStr(0, 52, buf);

    display.sendBuffer();
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
      display.drawStr(10, 30, "Collecting app...");
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
      display.drawStr(10, 30, "Collecting app...");
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
    String currentTempStr = String(app.humidity, 1) + "%";
    display.drawStr(100, 12, currentTempStr.c_str());
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
      display.drawStr(10, 30, "Collecting app...");
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
    //String currentTempStr = String(app.pressure_seaLevel, 1) + " hPa";
    //display.drawStr(100, 12, currentTempStr.c_str());
}

void draw_trend(const AppData& app) {
  int x = 40;
  int y = 50;
  switch(app.weatherTendency)
  {
    case 2:
      display.setFont(u8g2_font_open_iconic_weather_6x_t);
      display.drawGlyph(x, y, 69);
      display.drawStr(50, 64, "2");
      break;
    case 1:
      display.setFont(u8g2_font_open_iconic_weather_6x_t);
      display.drawGlyph(x, y, 65);	
      display.drawStr(60, 50, "1");
      break;
    case 0:
      display.setFont(u8g2_font_open_iconic_weather_6x_t);
      display.drawGlyph(x, y, 64);
      display.setFont(u8g2_font_6x12_tr);
      display.drawStr(50, 64, "0");
      break;
    case -1:
      display.setFont(u8g2_font_open_iconic_weather_6x_t);
      display.drawGlyph(x, y, 67);
      display.drawStr(50, 64, "-1");
      break;
    case -2:
      display.setFont(u8g2_font_open_iconic_embedded_6x_t);
      display.drawGlyph(x, y, 67);
      display.drawStr(50, 64, "-2");
      break;
  }
}

