#include "graphics.h"

#define HOUR_HAND_THICKNESS 8
#define MINUTE_HAND_THICKNESS 6

static Layer *hands_layer;

static Animation *animation;
static AnimationImplementation animation_implementation;
static bool init_animated = false;
int32_t hour_angle_target, minute_angle_target;

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

static int32_t calc_hour_angle(unsigned short hour, unsigned short minute) {
	return TRIG_MAX_ANGLE * ((hour % 12) + (minute / 60.0)) / 12.0;
}

static int32_t calc_minute_angle(unsigned short hour, unsigned short minute) {
	return TRIG_MAX_ANGLE * minute / 60.0;
}

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

static void animation_teardown(Animation *animation) {
	init_animated = true;
}

static void animation_update(Animation *animation, const AnimationProgress progress) {
	int32_t total = ANIMATION_NORMALIZED_MAX - ANIMATION_NORMALIZED_MIN;
	int32_t hour_angle_start = calc_hour_angle(10,8);
	int32_t minute_angle_start = calc_minute_angle(10,8);

	int32_t hour_diff = hour_angle_target-hour_angle_start;
	int32_t minute_diff = minute_angle_target-minute_angle_start;

	// always clockwise: if difference is negative, go the long way around
	if (hour_diff < 0) { hour_diff += TRIG_MAX_ANGLE; }
	if (minute_diff < 0) { minute_diff += TRIG_MAX_ANGLE; }

	int32_t hour_angle = hour_diff*progress/total + hour_angle_start;
	int32_t minute_angle = minute_diff*progress/total + minute_angle_start;

	gpath_rotate_to(hour_hand_path, hour_angle);
	gpath_rotate_to(minute_hand_path, minute_angle);
	layer_mark_dirty(hands_layer);
}

static void start_animation() {
	if (animation) {
		// avoid double animation
		animation_unschedule(animation);
		animation_destroy(animation);
	}
	animation = animation_create();
	animation_set_duration(animation, 400);
	animation_set_delay(animation, 0);
	animation_set_curve(animation, AnimationCurveEaseInOut);
	animation_implementation = (AnimationImplementation) {
		.update = animation_update,
		.teardown = animation_teardown
	};
	animation_set_implementation(animation, &animation_implementation);
	animation_schedule(animation);
}

void init_hands(Layer *layer) {
	GRect bounds = layer_get_bounds(layer);
	hands_layer = layer;

	hour_hand_path = gpath_create(&HOUR_HAND_POINTS);
	gpath_move_to(hour_hand_path, grect_center_point(&bounds));
	minute_hand_path = gpath_create(&MINUTE_HAND_POINTS);
	gpath_move_to(minute_hand_path, grect_center_point(&bounds));

	layer_set_update_proc(hands_layer, hands_update_proc);
}

void set_hour_hand_colour(GColor colour) {
	hour_hand_colour = colour;
}

void set_minute_hand_colour(GColor colour) {
	minute_hand_colour = colour;
}

void set_hands(unsigned short hour, unsigned short minute) {
	hour_angle_target = calc_hour_angle(hour, minute);
	minute_angle_target = calc_minute_angle(hour, minute);
	// APP_LOG(APP_LOG_LEVEL_DEBUG, "setting hands %d, %d", hour, minute);

	if (init_animated) {
		gpath_rotate_to(hour_hand_path, hour_angle_target);
		gpath_rotate_to(minute_hand_path, minute_angle_target);
		layer_mark_dirty(hands_layer);
	} else {
		start_animation();
	}
}

void destroy_hands() {
	gpath_destroy(hour_hand_path);
	gpath_destroy(minute_hand_path);
}
