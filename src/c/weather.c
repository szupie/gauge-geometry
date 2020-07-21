#include <pebble.h>
#include "weather.h"
#include "graphics.h"

uint32_t persist_temp_now = 1;
uint32_t persist_temp_min = 2;
uint32_t persist_temp_max = 3;

static GColor temp_range_colour;
static GColor temp_now_colour;

static Layer *temp_range_layer, *temp_now_layer;


static int minute_to_rad(int minute) {
	return DEG_TO_TRIGANGLE(minute*360/60);
}

static void temp_now_update_proc(Layer *layer, GContext *ctx) {
	if (persist_exists(persist_temp_now)) {
		int temp_now = persist_read_int(persist_temp_now);

		GRect bounds = layer_get_bounds(layer);
		int temp_angle = minute_to_rad(temp_now);

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
	if (persist_exists(persist_temp_min) && persist_exists(persist_temp_max)) {
		int temp_min = persist_read_int(persist_temp_min);
		int temp_max = persist_read_int(persist_temp_max);

		GRect bounds = layer_get_bounds(layer);
		graphics_context_set_fill_color(ctx, temp_range_colour);
		graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, TEMP_RANGE_WIDTH, minute_to_rad(temp_min), minute_to_rad(temp_max));
	}
}

void init_weather(Layer *range_layer, Layer *now_layer) {
	temp_range_layer = range_layer;
	temp_now_layer = now_layer;
	layer_set_update_proc(temp_range_layer, temp_range_update_proc);
	layer_set_update_proc(temp_now_layer, temp_now_update_proc);
}

void set_temp_range_colour(GColor colour) {
	temp_range_colour = colour;
}

void set_temp_now_colour(GColor colour) {
	temp_now_colour = colour;
}

void handle_weather_update(DictionaryIterator *iterator, void *context) {
	// Read tuples for data
	Tuple *temp_now_tuple = dict_find(iterator, MESSAGE_KEY_TEMP_NOW);
	Tuple *temp_min_tuple = dict_find(iterator, MESSAGE_KEY_TEMP_MIN);
	Tuple *temp_max_tuple = dict_find(iterator, MESSAGE_KEY_TEMP_MAX);

	// If temp range is available
	if(temp_min_tuple && temp_max_tuple) {
		int temp_min = (int)temp_min_tuple->value->int32;
		int temp_max = (int)temp_max_tuple->value->int32;

		persist_write_int(persist_temp_min, temp_min);
		persist_write_int(persist_temp_max, temp_max);

		layer_mark_dirty(temp_range_layer);
	}

	if(temp_now_tuple) {
		int temp_now = (int)temp_now_tuple->value->int32;
		persist_write_int(persist_temp_now, temp_now);
		layer_mark_dirty(temp_now_layer);
	}
}
