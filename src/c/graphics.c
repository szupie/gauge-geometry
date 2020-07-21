#include "graphics.h"
#include "big-digits.h"
#include "hands.h"
#include "weather.h"

#include "enamel.h"

static Layer* window_layer;

static TextLayer *s_day_layer, *s_date_layer, *s_day_shadow_layer, *s_date_shadow_layer;
static Layer *ticks_canvas, *hands_layer, *s_temp_range_layer, *s_temp_now_layer;

static BatteryChargeState battery_state;


static GColor bg_colour;
static GColor date_colour;
static GColor ticks_colour;
static int ticks_size;

static bool battery_gauge_enabled;
static bool temp_enabled;


static void ticks_update_proc(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);

	graphics_context_set_stroke_width(ctx, 3);
	graphics_context_set_stroke_color(ctx, ticks_colour);

	for (int i=0; i<12; i++) {
		int hour_angle = DEG_TO_TRIGANGLE(i*360/12);
		GPoint pos = gpoint_from_polar(grect_inset(bounds, GEdgeInsets(RING_INSET)), GOvalScaleModeFitCircle, hour_angle);

		if (!battery_gauge_enabled || i < battery_state.charge_percent*0.01*12) {
			graphics_context_set_fill_color(ctx, ticks_colour);
			graphics_fill_circle(ctx, pos, ticks_size);
		} else {
			if (ticks_size > 1) {
				graphics_context_set_fill_color(ctx, bg_colour);
				int ring_size = ticks_size;
				if (ticks_size > 2) {
					ring_size = ticks_size-1;
				}
				graphics_draw_circle(ctx, pos, ring_size);
				graphics_fill_circle(ctx, pos, ring_size);
				// check color TODO
			}
		}
	}
}

static void set_text_style(TextLayer *layer) {
	GFont custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_POPPINS_16));
	text_layer_set_background_color(layer, GColorClear);
	text_layer_set_font(layer, custom_font);
	text_layer_set_text_alignment(layer, GTextAlignmentCenter);
}


void load_window(Window *window) {
	window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	int font_size = 16;
	int center = 42;
	int bottom = bounds.size.h / 2 - font_size * 0;
	int width = 60;
	// int left = 5;
	int left = center - (width / 2);

	s_day_layer = text_layer_create(GRect(left, bottom - (font_size*2), width, font_size*1.5));
	s_day_shadow_layer = text_layer_create(GRect(left+TEXT_SHADOW_OFFSET, bottom - (font_size*2)+TEXT_SHADOW_OFFSET, width, font_size*1.5));

	s_date_layer = text_layer_create(GRect(left, bottom - font_size, width, font_size*1.5));
	s_date_shadow_layer = text_layer_create(GRect(left+TEXT_SHADOW_OFFSET, bottom - font_size + TEXT_SHADOW_OFFSET, width, font_size*1.5));

	set_text_style(s_day_layer);
	set_text_style(s_day_shadow_layer);

	set_text_style(s_date_layer);
	set_text_style(s_date_shadow_layer);

	// temperature
	s_temp_range_layer = layer_create(bounds);
	s_temp_now_layer = layer_create(bounds);
	init_weather(s_temp_range_layer, s_temp_now_layer);

	// create ticks
	ticks_canvas = layer_create(bounds);
	layer_set_update_proc(ticks_canvas, ticks_update_proc);

	init_digits(ticks_canvas);

	hands_layer = layer_create(bounds);
	init_hands(bounds, hands_layer);

	layer_add_child(window_layer, s_temp_range_layer);
	layer_add_child(window_layer, ticks_canvas);
	layer_add_child(window_layer, s_temp_now_layer);
	layer_add_child(window_layer, hands_layer);

	layer_add_child(window_layer, text_layer_get_layer(s_day_shadow_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_date_shadow_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_day_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

	update_style(NULL);
}

void update_style() {
	bg_colour = enamel_get_S_BG_COLOUR();
	set_digits_colour(enamel_get_S_TIME_COLOUR());
	date_colour = enamel_get_S_DATE_COLOUR();
	set_hour_hand_colour(enamel_get_S_HOUR_HAND_COLOUR());
	set_minute_hand_colour(enamel_get_S_MINUTE_HAND_COLOUR());
	ticks_colour = enamel_get_S_TICKS_COLOUR();
	ticks_size = enamel_get_S_TICKS_SIZE();

	battery_gauge_enabled = enamel_get_S_BATTERY_GAUGE_ENABLED();
	temp_enabled = enamel_get_S_TEMP_ENABLED();
	set_temp_range_colour(enamel_get_S_TEMP_RANGE_COLOUR());
	set_temp_now_colour(enamel_get_S_TEMP_NOW_COLOUR());

	layer_set_hidden(s_temp_range_layer, !temp_enabled);
	layer_set_hidden(s_temp_now_layer, !temp_enabled);
	
	layer_mark_dirty(s_temp_range_layer);
	layer_mark_dirty(s_temp_now_layer);
	layer_mark_dirty(hands_layer);
	layer_mark_dirty(ticks_canvas);

	window_set_background_color(layer_get_window(window_layer), bg_colour);

	text_layer_set_text_color(s_day_layer, date_colour);
	text_layer_set_text_color(s_date_layer, date_colour);
	text_layer_set_text_color(s_day_shadow_layer, bg_colour);
	text_layer_set_text_color(s_date_shadow_layer, bg_colour);
}

void update_time(unsigned short hour, unsigned short minute) {
	set_digits_hour(hour);
	set_digits_minute(minute);

	set_hands(hour, minute);
}

void update_day_of_week(char *day) {
	text_layer_set_text(s_day_layer, day);
	text_layer_set_text(s_day_shadow_layer, day);
}

void update_date_month(char *date) {
	text_layer_set_text(s_date_layer, date);
	text_layer_set_text(s_date_shadow_layer, date);
}

void handle_battery_update(BatteryChargeState charge_state) {
	battery_state = charge_state;
	layer_mark_dirty(ticks_canvas);
}

GColor get_bg_colour() {
	return bg_colour;
}

GRect get_window_bounds() {
	return layer_get_bounds(window_layer);
}


void destroy_layers() {
	destroy_digits_layers();

	// Destroy TextLayer
	text_layer_destroy(s_day_layer);
	text_layer_destroy(s_day_shadow_layer);
	text_layer_destroy(s_date_layer);
	text_layer_destroy(s_date_shadow_layer);

	layer_destroy(ticks_canvas);
	layer_destroy(hands_layer);
	destroy_hands();
}
