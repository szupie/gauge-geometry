#pragma once
#include <pebble.h>

#if defined(PBL_ROUND)
#define RING_INSET 8
#define TEMP_RANGE_WIDTH 12
#elif defined(PBL_RECT)
#define RING_INSET 4
#define TEMP_RANGE_WIDTH 8
#endif
#define TEMP_NOW_RADIUS 9
#define TEMP_NOW_STROKE 7
#define TEXT_SHADOW_OFFSET 1

#define EMERY_LEADING 14


void load_window(Window *window);
void update_style();
void update_time(unsigned short hour, unsigned short minute);
void update_day_of_week(char *day);
void update_date_month(char *date);
GColor get_bg_colour();
GColor get_stroke_colour_for_fill(GColor fill);
GPoint get_point_at_rect_perim(int angle, GRect frame);
void destroy_layers();
