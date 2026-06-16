#include "button.h"
#include "app_data.h"
#include <Arduino.h>

void ui_handleButton() {

    if (button_wasPressed()) {
      app.lastButtonTime = millis();
      app.currentScreen = uiScreen(app.currentScreen + 1);
      if (app.currentScreen >= SCREEN_SCREENSAVER) {
            app.currentScreen = SCREEN_MAIN;
      }
    }      
}

void ui_backToMain() {
    if (millis() - app.lastButtonTime > 300000UL && app.currentScreen != SCREEN_MAIN && app.currentScreen != SCREEN_SCREENSAVER) {
        app.currentScreen = SCREEN_MAIN;
    }
}

void ui_screenSaver() {
    if (millis() - app.lastButtonTime > 600000) {
        app.currentScreen = SCREEN_SCREENSAVER;
    }
}

void ui_update() {
    ui_handleButton();
    ui_backToMain();
    ui_screenSaver();
}