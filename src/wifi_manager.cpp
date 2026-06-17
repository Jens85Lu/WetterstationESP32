#include "secrets.h"
#include <WiFi.h>
#include <Arduino.h>
#include "app_data.h"

void wifi_init()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int i = 0;   // <- NICHT static

    app.wifiConnected = false;

    Serial.print("Connecting");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");

        i++;

        if (i >= 10)   // ~5 Sekunden Timeout
        {
            Serial.println("\nNo WiFi connection.");
            return;
        }
    }

    Serial.println();
    Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());

    app.wifiConnected = true;
}

void wifi_update() {
    if (WiFi.status() == WL_CONNECTED) {
        app.wifiConnected = true;
        return;
    }

    app.wifiConnected = false;

    // alle 60 Sekunden neu versuchen
    static unsigned long lastWLANSearch = 0;
    
    if (millis() - lastWLANSearch > 60000 && !app.wifiConnected) {
        lastWLANSearch = millis();
        Serial.println("WiFI reconnect...");
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        unsigned long start = millis();

        while (WiFi.status() != WL_CONNECTED &&
            millis() - start < 10000)
        {
            delay(200);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\nWiFi connected");
            Serial.println(WiFi.localIP());
            app.wifiConnected = true;
        }
        else
        {
            Serial.println("\nWiFi failed");
            app.wifiConnected = false;
        }
    }
}