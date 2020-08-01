#pragma once
#include <pebble.h>

#define RING_INSET 8
#define TEMP_RANGE_WIDTH 12
#define TEMP_NOW_RADIUS 6
#define TEMP_NOW_STROKE 6
#define TEXT_SHADOW_OFFSET 1


void load_window(Window *window);
void update_style();
void update_time(unsigned short hour, unsigned short minute);
void update_day_of_week(char *day);
void update_date_month(char *date);
void handle_battery_update(BatteryChargeState charge_state);
GColor get_bg_colour();
void destroy_layers();
