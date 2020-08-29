#include "graphics.h"
#include "big-digits.h"
#include "hands.h"
#include "weather.h"

#include "enamel.h"

static Layer* window_layer;

static TextLayer *day_text_layer, *date_text_layer, *day_shadow_text_layer, *date_shadow_text_layer_a, *date_shadow_text_layer_b;
static Layer *date_group_layer, *digits_layer, *ticks_canvas, *hands_layer, *temp_range_layer, *temp_now_layer;

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

static void init_text_style(TextLayer *layer) {
	GFont custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_POPPINS_16));
	text_layer_set_background_color(layer, GColorClear);
	text_layer_set_font(layer, custom_font);
	text_layer_set_text_alignment(layer, GTextAlignmentCenter);
}

// adjust date position to avoid overlapping with big digit for hour
static void update_date_group_position(unsigned short hour) {
	GRect bounds = layer_get_bounds(window_layer);
	GRect frame = layer_get_frame(date_group_layer);

	int center = bounds.size.w / 2;
	int left = center/2 - frame.size.w/2;

	if (hour >= 20) {
		frame.origin.x = 5;
	} else if (hour >= 10) {
		frame.origin.x = left - 8;
	} else {
		frame.origin.x = left;
	}
	layer_set_frame(date_group_layer, frame);
}

void load_window(Window *window) {
	window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	// create day of week and date text layers
	int line_height = 16;
	int bottom = bounds.size.h / 2 - 3;
	int width = 60;

	date_group_layer = layer_create(GRect(0, bottom - (line_height*2), width, line_height*3));

	day_text_layer = text_layer_create(GRect(0, 0, width, line_height*1.5));
	day_shadow_text_layer = text_layer_create(GRect(TEXT_SHADOW_OFFSET, TEXT_SHADOW_OFFSET, width, line_height*1.5));

	date_text_layer = text_layer_create(GRect(0, line_height, width, line_height*1.5));
	date_shadow_text_layer_a = text_layer_create(GRect(TEXT_SHADOW_OFFSET, line_height + TEXT_SHADOW_OFFSET, width, line_height*1.5));
	date_shadow_text_layer_b = text_layer_create(GRect(TEXT_SHADOW_OFFSET, line_height - TEXT_SHADOW_OFFSET, width, line_height*1.5));

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

	// create temperature gauge
	temp_range_layer = layer_create(bounds);
	temp_now_layer = layer_create(bounds);
	init_weather(temp_range_layer, temp_now_layer);

	// create ticks
	ticks_canvas = layer_create(bounds);
	layer_set_update_proc(ticks_canvas, ticks_update_proc);

	// create big digits
	digits_layer = layer_create(bounds);
	init_digits(digits_layer);

	// create hands
	hands_layer = layer_create(bounds);
	init_hands(hands_layer);

	// add layers, foreground last
	layer_add_child(window_layer, temp_range_layer);
	layer_add_child(window_layer, digits_layer);
	layer_add_child(window_layer, ticks_canvas);
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
	set_hour_hand_colour(enamel_get_HOUR_HAND_COLOUR());
	set_minute_hand_colour(enamel_get_MINUTE_HAND_COLOUR());
	ticks_colour = enamel_get_TICKS_COLOUR();
	ticks_size = enamel_get_TICKS_SIZE();

	battery_gauge_enabled = enamel_get_BATTERY_GAUGE_ENABLED();
	temp_enabled = enamel_get_TEMP_ENABLED();
	check_temp_unit_change();
	set_temp_range_colour(enamel_get_TEMP_RANGE_COLOUR());
	set_temp_now_colour(enamel_get_TEMP_NOW_COLOUR());
	layer_set_hidden(temp_range_layer, !temp_enabled);
	layer_set_hidden(temp_now_layer, !temp_enabled);
	
	layer_mark_dirty(hands_layer);
	layer_mark_dirty(ticks_canvas);

	window_set_background_color(layer_get_window(window_layer), bg_colour);

	text_layer_set_text_color(day_text_layer, date_colour);
	text_layer_set_text_color(date_text_layer, date_colour);
	text_layer_set_text_color(day_shadow_text_layer, bg_colour);
	text_layer_set_text_color(date_shadow_text_layer_a, bg_colour);
	text_layer_set_text_color(date_shadow_text_layer_b, bg_colour);
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

void handle_battery_update(BatteryChargeState charge_state) {
	battery_state = charge_state;
	layer_mark_dirty(ticks_canvas);
}

GColor get_bg_colour() {
	return bg_colour;
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
	layer_destroy(ticks_canvas);
	layer_destroy(hands_layer);
	layer_destroy(date_group_layer);
	layer_destroy(temp_range_layer);
	layer_destroy(temp_now_layer);
}
