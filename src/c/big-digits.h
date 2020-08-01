#pragma once

void init_digits(Layer *parent_layer);
void set_digits(unsigned short hour, unsigned short minute);
void set_digits_hour(unsigned short value);
void set_digits_minute(unsigned short value);
void set_digits_colour(GColor colour);
void destroy_digits();