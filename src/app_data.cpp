#include "app_data.h"
#include "sensors.h"


AppData app;

void update_appData() {
    app.temp = getTemp();
    app.humidity = getHumidity();
    app.pressure = getPressure();
    app.bmpTemperature = getBmpTemperature();
}