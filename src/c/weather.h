#pragma once
#include <pebble.h>

void init_weather(Layer *range_layer, Layer *now_layer);
void set_temp_range_colour(GColor colour);
void set_temp_now_colour(GColor colour);
void check_temp_unit_change(char new_temp_unit);
void update_temp_range(int min, int max);
void update_temp_now(int now);
void enable_temp(bool enabled);
void handle_weather_update(DictionaryIterator *iterator, void *context);
