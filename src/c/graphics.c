#include "graphics.h"
#include "big-digits.h"
#include "hands.h"
#include "ticks.h"
#include "weather.h"

#include "enamel.h"

static Layer* window_layer;

static TextLayer *day_text_layer, *date_text_layer, *day_shadow_text_layer, *date_shadow_text_layer_a, *date_shadow_text_layer_b;
static Layer *date_group_layer, *digits_layer, *ticks_layer, *hands_layer, *temp_range_layer, *temp_now_layer;

static GColor bg_colour;
static GColor date_colour;

static void init_text_style(TextLayer *layer) {
	GFont custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_POPPINS_16));
	text_layer_set_background_color(layer, GColorClear);
	text_layer_set_font(layer, custom_font);
	text_layer_set_text_alignment(layer, GTextAlignmentCenter);
	#if defined(PBL_RECT)
	text_layer_set_text_alignment(layer, GTextAlignmentLeft);
	#endif
}

// adjust date position to avoid overlapping with big digit for hour
static void update_date_group_position(unsigned short hour) {
	GRect frame = layer_get_frame(date_group_layer);

	#if defined(PBL_ROUND)
	GRect bounds = layer_get_bounds(window_layer);
	int center = bounds.size.w / 2;
	int left = center/2 - frame.size.w/2;

	if (hour >= 20) {
		frame.origin.x = left - 10;
	} else if (hour >= 10) {
		frame.origin.x = left - 8;
	} else {
		frame.origin.x = left;
	}
	#elif defined(PBL_RECT)
	if (hour >= 10) {
		frame.origin.x = 8;
	} else {
		frame.origin.x = 15;
	}
	#endif

	layer_set_frame(date_group_layer, frame);
}

static void init_text_layers(GRect bounds) {
	// create day of week and date text layers
	int line_height = 16;
	int bottom = bounds.size.h / 2 - 3;
	int width = 80;
	#if defined(PBL_ROUND)
	int top = bottom - (line_height*2);
	#elif defined(PBL_RECT)
	int top = bottom - (line_height*3);
	#endif

	date_group_layer = layer_create(GRect(0, top, width, line_height*3.5));

	day_text_layer = text_layer_create(GRect(0, 0, width, line_height*1.5));
	day_shadow_text_layer = text_layer_create(GRect(TEXT_SHADOW_OFFSET, TEXT_SHADOW_OFFSET, width, line_height*1.5));

	date_text_layer = text_layer_create(GRect(0, line_height, width, line_height*2.5));
	date_shadow_text_layer_a = text_layer_create(GRect(TEXT_SHADOW_OFFSET, line_height + TEXT_SHADOW_OFFSET, width, line_height*2.5));
	date_shadow_text_layer_b = text_layer_create(GRect(TEXT_SHADOW_OFFSET, line_height - TEXT_SHADOW_OFFSET, width, line_height*2.5));

	init_text_style(day_text_layer);
	init_text_style(day_shadow_text_layer);
	init_text_style(date_text_layer);
	init_text_style(date_shadow_text_layer_a);
	init_text_style(date_shadow_text_layer_b);

	layer_add_child(date_group_layer, text_layer_get_layer(day_shadow_text_layer));
	layer_add_child(date_group_layer, text_layer_get_layer(date_shadow_text_layer_a));
	layer_add_child(date_group_layer, text_layer_get_layer(date_shadow_text_layer_b));
	layer_add_child(date_group_layer, text_layer_get_layer(day_text_layer));
	layer_add_child(date_group_layer, text_layer_get_layer(date_text_layer));
}

static enum HandShape enamel_to_hand_shape(HANDS_SHAPEValue enamel_value) {
	switch (enamel_value) {
		case HANDS_SHAPE_BAGUETTE:
			return BAGUETTE;
		case HANDS_SHAPE_PENCIL:
			return PENCIL;
		case HANDS_SHAPE_BREGUET:
			return BREGUET;
		case HANDS_SHAPE_SWISS_RAIL:
			return SWISSRAIL;
		default:
		case HANDS_SHAPE_DAUPHINE:
			return DAUPHINE;
	}
}

void load_window(Window *window) {
	window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	init_text_layers(bounds);

	// create temperature gauge
	temp_range_layer = layer_create(bounds);
	temp_now_layer = layer_create(bounds);
	init_weather(temp_range_layer, temp_now_layer);

	// create ticks
	ticks_layer = layer_create(bounds);
	init_ticks(ticks_layer);

	// create big digits
	digits_layer = layer_create(bounds);
	init_digits(digits_layer);

	// create hands
	hands_layer = layer_create(bounds);
	init_hands(hands_layer);

	// add layers, foreground last
	layer_add_child(window_layer, temp_range_layer);
	layer_add_child(window_layer, digits_layer);
	layer_add_child(window_layer, ticks_layer);
	layer_add_child(window_layer, temp_now_layer);
	layer_add_child(window_layer, hands_layer);
	layer_add_child(window_layer, date_group_layer);

	// set style from settings
	update_style();

}

void update_style() {
	bg_colour = enamel_get_BG_COLOUR();
	set_digits_colour(enamel_get_TIME_COLOUR());
	date_colour = enamel_get_DATE_COLOUR();
	update_hands_settings(
		enamel_get_HOUR_HAND_COLOUR(),
		enamel_get_MINUTE_HAND_COLOUR(),
		enamel_to_hand_shape(enamel_get_hands_shape())
	);

	update_tick_settings(
		enamel_get_TICKS_COLOUR(),
		enamel_get_TICKS_SIZE(),
		enamel_get_BATTERY_GAUGE_ENABLED()
	);
	enable_temp(enamel_get_TEMP_ENABLED());
	check_temp_unit_change();
	set_temp_range_colour(enamel_get_TEMP_RANGE_COLOUR());
	set_temp_now_colour(enamel_get_TEMP_NOW_COLOUR());
	
	window_set_background_color(layer_get_window(window_layer), bg_colour);

	text_layer_set_text_color(day_text_layer, date_colour);
	text_layer_set_text_color(date_text_layer, date_colour);
	GColor date_shadow = get_stroke_colour_for_fill(date_colour);
	text_layer_set_text_color(day_shadow_text_layer, date_shadow);
	text_layer_set_text_color(date_shadow_text_layer_a, date_shadow);
	text_layer_set_text_color(date_shadow_text_layer_b, date_shadow);
}

void update_time(unsigned short hour, unsigned short minute) {
	set_digits(hour, minute);

	update_date_group_position(hour);

	set_hands(hour, minute);
}

void update_day_of_week(char *day) {
	text_layer_set_text(day_text_layer, day);
	text_layer_set_text(day_shadow_text_layer, day);
}

void update_date_month(char *date) {
	text_layer_set_text(date_text_layer, date);
	text_layer_set_text(date_shadow_text_layer_a, date);
	text_layer_set_text(date_shadow_text_layer_b, date);
}

void destroy_layers() {
	destroy_digits();
	destroy_hands();

	text_layer_destroy(day_text_layer);
	text_layer_destroy(day_shadow_text_layer);
	text_layer_destroy(date_text_layer);
	text_layer_destroy(date_shadow_text_layer_a);
	text_layer_destroy(date_shadow_text_layer_b);

	layer_destroy(digits_layer);
	layer_destroy(ticks_layer);
	layer_destroy(hands_layer);
	layer_destroy(date_group_layer);
	layer_destroy(temp_range_layer);
	layer_destroy(temp_now_layer);
}


// Share helper functions

GColor get_bg_colour() {
	return bg_colour;
}

// Use bg colour as stroke on colour displays. Otherwise get legible colour
GColor get_stroke_colour_for_fill(GColor fill) {
	#ifdef PBL_COLOR
	return get_bg_colour();
	#else
	return gcolor_legible_over(fill);
	#endif
}

GPoint get_point_at_rect_perim(int angle, GRect frame) {
	// cornerAngle: angle between top right corner and top center
	// Get complement since atan2 is measured anticlockwise from +x axis, 
	// while pebble angles are measured clockwise from +y axis
	int32_t cornerAngle = TRIG_MAX_ANGLE/4 - atan2_lookup(frame.size.h, frame.size.w);
	angle = (angle + TRIG_MAX_ANGLE) % TRIG_MAX_ANGLE; // normalise angle
	int topRightAngle = cornerAngle;
	int topLeftAngle = TRIG_MAX_ANGLE - cornerAngle;
	int bottomRightAngle = TRIG_MAX_ANGLE/2 - cornerAngle;
	int bottomLeftAngle = TRIG_MAX_ANGLE/2 + cornerAngle;
	bool isTopEdge = angle < topRightAngle || angle > topLeftAngle;
	bool isBottomEdge = angle > bottomRightAngle && angle < bottomLeftAngle;

	GPoint center = GPoint(frame.origin.x + frame.size.w/2, frame.origin.y + frame.size.h/2);

	if (isTopEdge || isBottomEdge) {
		// top or bottom edges
		int8_t topBottomSign = cos_lookup(angle)/abs(cos_lookup(angle));
		int16_t yEdge = center.y - topBottomSign*frame.size.h/2;

		return GPoint(center.x + topBottomSign*(frame.size.h/2)*sin_lookup(angle)/cos_lookup(angle), yEdge);
	} else {
		// left or right edges
		int8_t leftRightSign = sin_lookup(angle)/abs(sin_lookup(angle));
		int16_t xEdge = center.x + leftRightSign*frame.size.w/2;

		return GPoint(xEdge, center.y - leftRightSign*(frame.size.w/2)*cos_lookup(angle)/sin_lookup(angle));
	}
}
