#include "graphics.h"

#define HOUR_HAND_THICKNESS 8
#define MINUTE_HAND_THICKNESS 6

static Layer *hands_layer;

static const GPathInfo HOUR_HAND_POINTS = {
	4, (GPoint []){
		{0, HOUR_HAND_THICKNESS},
		{-HOUR_HAND_THICKNESS, 0},
		{0, -56},
		{HOUR_HAND_THICKNESS, 0}
	}
};
static const GPathInfo MINUTE_HAND_POINTS = {
	4, (GPoint []){
		{0, MINUTE_HAND_THICKNESS},
		{-MINUTE_HAND_THICKNESS, 0},
		{0, -80},
		{MINUTE_HAND_THICKNESS, 0}
	}
};

static GPath *minute_hand_path, *hour_hand_path;

static GColor hour_hand_colour;
static GColor minute_hand_colour;

static void hands_update_proc(Layer *layer, GContext *ctx) {
	graphics_context_set_fill_color(ctx, minute_hand_colour);
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	gpath_draw_outline(ctx, minute_hand_path);
	gpath_draw_filled(ctx, minute_hand_path);

	graphics_context_set_fill_color(ctx, hour_hand_colour);
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	gpath_draw_outline(ctx, hour_hand_path);
	gpath_draw_filled(ctx, hour_hand_path);
}

void init_hands(GRect bounds, Layer *layer) {
	hour_hand_path = gpath_create(&HOUR_HAND_POINTS);
	gpath_move_to(hour_hand_path, grect_center_point(&bounds));
	minute_hand_path = gpath_create(&MINUTE_HAND_POINTS);
	gpath_move_to(minute_hand_path, grect_center_point(&bounds));

	hands_layer = layer;

	layer_set_update_proc(hands_layer, hands_update_proc);
}

void set_hour_hand_colour(GColor colour) {
	hour_hand_colour = colour;
}

void set_minute_hand_colour(GColor colour) {
	minute_hand_colour = colour;
}

void set_hands(unsigned short hour, unsigned short minute) {
	gpath_rotate_to(hour_hand_path, (TRIG_MAX_ANGLE * (((hour % 12) * 6) + (minute / 10))) / (12 * 6));
	gpath_rotate_to(minute_hand_path, TRIG_MAX_ANGLE * minute / 60);
	layer_mark_dirty(hands_layer);
}

void destroy_hands() {
	gpath_destroy(hour_hand_path);
	gpath_destroy(minute_hand_path);
}
