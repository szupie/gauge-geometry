#pragma once
#include <pebble.h>

void init_hands(Layer *layer);
void set_hour_hand_colour(GColor colour);
void set_minute_hand_colour(GColor colour);
void set_hands(unsigned short hour, unsigned short minute);
void destroy_hands();
