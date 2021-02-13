#pragma once
#include <pebble.h>

void init_ticks(Layer *layer);
void update_battery_ticks(BatteryChargeState charge_state);
void animate_charging_indicator();
void update_tick_settings(GColor colour, int size, bool battery_enabled);
