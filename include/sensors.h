#pragma once

void sensors_init();
void sensors_read();

float getTemp();
float getHumidity();
float getPressure();
float getBmpTemperature();