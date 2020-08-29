#include <pebble.h>
#include "weather.h"
#include "graphics.h"

#include "enamel.h"

uint32_t persist_temp_now = 1;
uint32_t persist_temp_min = 2;
uint32_t persist_temp_max = 3;

static int temp_min, temp_max, temp_now;
static bool temp_range_defined = false;
static bool temp_now_defined = false;

static char temp_unit;

static GColor temp_range_colour;
static GColor temp_now_colour;
static bool temp_range_colour_defined = false;
static bool temp_now_colour_defined = false;

static Layer *temp_range_layer, *temp_now_layer;

// Celsius to angle. Minute marks correspond to degrees
// Gauge on clockface goes clockwise from 0 to 60C
static int c_to_rad(int degree) {
	return DEG_TO_TRIGANGLE(degree*360/60);
}
// Fahrenheit to angle. Every hour mark corresponds to 10F
// Gauge on clockface goes clockwise from 0 to 120F
static int f_to_rad(int degree) {
	return DEG_TO_TRIGANGLE(degree*360/120);
}

static int degree_to_rad(int degree) {
	switch (temp_unit) {
		case 'f':
			return f_to_rad(degree);
		case 'c':
		default:
			return c_to_rad(degree);
	}
}

static void temp_now_update_proc(Layer *layer, GContext *ctx) {
	if (temp_now_defined && temp_now_colour_defined) {
		GRect bounds = layer_get_bounds(layer);
		int temp_angle = degree_to_rad(temp_now);

		// graphics_context_set_fill_color(ctx, GColorDarkCandyAppleRed);
		// graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, RING_INSET*2, temp_angle, temp_angle);

		GPoint pos = gpoint_from_polar(grect_inset(bounds, GEdgeInsets(RING_INSET)), GOvalScaleModeFitCircle, temp_angle);
		graphics_context_set_fill_color(ctx, get_bg_colour());
		graphics_context_set_stroke_color(ctx, temp_now_colour);
		graphics_context_set_stroke_width(ctx, TEMP_NOW_STROKE);
		graphics_fill_circle(ctx, pos, TEMP_NOW_RADIUS);
		graphics_draw_circle(ctx, pos, TEMP_NOW_RADIUS);
	}
}

static void temp_range_update_proc(Layer *layer, GContext *ctx) {
	if (temp_range_defined && temp_range_colour_defined) {
		GRect bounds = layer_get_bounds(layer);
		graphics_context_set_fill_color(ctx, temp_range_colour);
		graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, TEMP_RANGE_WIDTH, degree_to_rad(temp_min), degree_to_rad(temp_max));
	}
}

void init_weather(Layer *range_layer, Layer *now_layer) {
	temp_range_layer = range_layer;
	temp_now_layer = now_layer;
	layer_set_update_proc(temp_range_layer, temp_range_update_proc);
	layer_set_update_proc(temp_now_layer, temp_now_update_proc);

	temp_unit = enamel_get_TEMP_UNIT()[0];

	if (persist_exists(persist_temp_min) && persist_exists(persist_temp_max)) {
		update_temp_range(
			persist_read_int(persist_temp_min),
			persist_read_int(persist_temp_max)
		);
	}
	if (persist_exists(persist_temp_now)) {
		update_temp_now(
			persist_read_int(persist_temp_now)
		);
	}
}

void set_temp_range_colour(GColor colour) {
	if (!temp_range_colour_defined || !gcolor_equal(temp_range_colour, colour)) {
		temp_range_colour = colour;
		layer_mark_dirty(temp_range_layer);
	}
	temp_range_colour_defined = true;
}

void set_temp_now_colour(GColor colour) {
	if (!temp_now_colour_defined || !gcolor_equal(temp_now_colour, colour)) {
		temp_now_colour = colour;
		layer_mark_dirty(temp_now_layer);
	}
	temp_now_colour_defined = true;
}

void check_temp_unit_change() {
	char latest_unit = enamel_get_TEMP_UNIT()[0];
	// Clear cached temps when switching units as old values become nonsense
	if (latest_unit != temp_unit) {
		persist_delete(persist_temp_now);
		persist_delete(persist_temp_min);
		persist_delete(persist_temp_max);
		temp_range_defined = false;
		temp_now_defined = false;

		layer_mark_dirty(temp_range_layer);
		layer_mark_dirty(temp_now_layer);
	}
	temp_unit = latest_unit;
}

void update_temp_range(int min, int max) {
	temp_min = min;
	temp_max = max;
	temp_range_defined = true;
	layer_mark_dirty(temp_range_layer);
}

void update_temp_now(int now) {
	temp_now = now;
	temp_now_defined = true;
	layer_mark_dirty(temp_now_layer);
}

void handle_weather_update(DictionaryIterator *iterator, void *context) {
	// Read tuples for data
	Tuple *temp_now_tuple = dict_find(iterator, MESSAGE_KEY_TEMP_NOW);
	Tuple *temp_min_tuple = dict_find(iterator, MESSAGE_KEY_TEMP_MIN);
	Tuple *temp_max_tuple = dict_find(iterator, MESSAGE_KEY_TEMP_MAX);

	// If temp range is available
	if(temp_min_tuple && temp_max_tuple) {
		update_temp_range(
			(int)temp_min_tuple->value->int32,
			(int)temp_max_tuple->value->int32
		);
		persist_write_int(persist_temp_min, temp_min);
		persist_write_int(persist_temp_max, temp_max);
	}

	if(temp_now_tuple) {
		update_temp_now((int)temp_now_tuple->value->int32);
		persist_write_int(persist_temp_now, temp_now);
	}
}
