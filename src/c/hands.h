#pragma once
#include <pebble.h>
#include "hand_shapes.h"

void init_hands(Layer *layer);
void update_hands_settings(GColor hour, GColor minute, enum HandShape shape);
void set_hands(unsigned short hour, unsigned short minute);
void destroy_hands();
