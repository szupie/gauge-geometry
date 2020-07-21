#pragma once
#include <pebble.h>

void init_hands(GRect bounds, Layer *layer);
void set_hour_hand_colour(GColor colour);
void set_minute_hand_colour(GColor colour);
void set_hands(unsigned short hour, unsigned short minute);
void destroy_hands();
