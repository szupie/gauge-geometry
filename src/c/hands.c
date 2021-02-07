#include "graphics.h"
#include "hands.h"

static Layer *hands_layer;

static Animation *animation;
static AnimationImplementation animation_implementation;
static bool init_animated = false;
int32_t hour_angle_target, minute_angle_target;

enum HandShape hand_shape;
static GPath *minute_hand_path, *hour_hand_path;

static GColor hour_hand_colour;
static GColor minute_hand_colour;

static int32_t calc_hour_angle(unsigned short hour, unsigned short minute) {
	return TRIG_MAX_ANGLE * ((hour % 12) + (minute / 60.0)) / 12.0;
}

static int32_t calc_minute_angle(unsigned short minute) {
	return TRIG_MAX_ANGLE * minute / 60.0;
}

static void set_hand_paths(enum HandShape shape) {
	switch (shape) {
		case BAGUETTE: {
			hour_hand_path = gpath_create(&BAGUETTE_HOUR_POINTS);
			minute_hand_path = gpath_create(&BAGUETTE_MINUTE_POINTS);
			break;
		}
		case PENCIL: {
			hour_hand_path = gpath_create(&PENCIL_HOUR_POINTS);
			minute_hand_path = gpath_create(&PENCIL_MINUTE_POINTS);
			break;
		}
		default:
		case DAUPHINE: {
			hour_hand_path = gpath_create(&DAUPHINE_HOUR_POINTS);
			minute_hand_path = gpath_create(&DAUPHINE_MINUTE_POINTS);
			break;
		}
	}

	GRect bounds = layer_get_bounds(hands_layer);
	gpath_move_to(hour_hand_path, grect_center_point(&bounds));
	gpath_move_to(minute_hand_path, grect_center_point(&bounds));
}

// Draw hand by repeatedly drawing same path across transverse axis
// Point at center created by pushing inward lines further from center
static void draw_pencil_hand(GPath * path, int width, int density, GColor colour, GPoint origin, GContext *ctx) {
	float pointy_factor = 4.2;
	// move hand toward rim to prevent center end from jutting past center axle
	int outset_distance = width*1.5;

	int32_t angle = path->rotation;
	float h_component = cos_lookup(angle)/(float)TRIG_MAX_RATIO;
	float v_component = sin_lookup(angle)/(float)TRIG_MAX_RATIO;

	graphics_context_set_stroke_width(ctx, 1);
	graphics_context_set_stroke_color(ctx, colour);

	for (int i=0; i<width*density; i++) {
		float side_offset = (i+1)/(float)density - width/2.0;
		float offcenterness = abs((i+1) - width*density/2.0)/(float)density;
		float pointy_offset = pointy_factor*offcenterness - outset_distance;

		float offset_x = h_component*side_offset - v_component*pointy_offset;
		float offset_y = v_component*side_offset + h_component*pointy_offset;

		gpath_move_to(path, GPoint(origin.x+offset_x, origin.y+offset_y));
		gpath_draw_outline_open(ctx, path);
	}
}

static void draw_pencil_hands(GPoint center, GContext *ctx) {
	int width = 5;
	int density = 4;

	int stroke_density = 2;
	int stroke_outset = 2;

	int axle_radius = width+1;

	int32_t hour_angle = hour_hand_path->rotation;
	int32_t minute_angle = minute_hand_path->rotation;

	// minute hand stroke (same path drawn thicker and outset) then fill
	GPoint minute_stroke_origin = { 
		center.x+sin_lookup(minute_angle)*stroke_outset/TRIG_MAX_RATIO, 
		center.y-cos_lookup(minute_angle)*stroke_outset/TRIG_MAX_RATIO
	};
	draw_pencil_hand(minute_hand_path, width+2, stroke_density, get_bg_colour(), minute_stroke_origin, ctx);
	draw_pencil_hand(minute_hand_path, width, density, minute_hand_colour, center, ctx);

	// draw native thick stroke to cover any missing areas
	// inset path to hide rounded ends
	GPoint minute_native_origin = {
		center.x-sin_lookup(minute_angle)*width/TRIG_MAX_RATIO, 
		center.y+cos_lookup(minute_angle)*width/TRIG_MAX_RATIO
	};
	gpath_move_to(minute_hand_path, minute_native_origin);
	graphics_context_set_stroke_width(ctx, width);
	graphics_context_set_stroke_color(ctx, minute_hand_colour);
	gpath_draw_outline_open(ctx, minute_hand_path);

	// center axle stroke
	graphics_context_set_stroke_width(ctx, 1);
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	graphics_draw_circle(ctx, center, axle_radius+1);

	// hour hand stroke then fill
	GPoint hour_stroke_origin = { 
		center.x+sin_lookup(hour_angle)*stroke_outset/TRIG_MAX_RATIO, 
		center.y-cos_lookup(hour_angle)*stroke_outset/TRIG_MAX_RATIO
	};
	draw_pencil_hand(hour_hand_path, width+2, stroke_density, get_bg_colour(), hour_stroke_origin, ctx);
	draw_pencil_hand(hour_hand_path, width, density, hour_hand_colour, center, ctx);

	// native thick stroke for coverage
	GPoint hour_native_origin = {
		center.x-sin_lookup(hour_angle)*width/TRIG_MAX_RATIO, 
		center.y+cos_lookup(hour_angle)*width/TRIG_MAX_RATIO
	};
	gpath_move_to(hour_hand_path, hour_native_origin);
	graphics_context_set_stroke_width(ctx, width);
	graphics_context_set_stroke_color(ctx, hour_hand_colour);
	gpath_draw_outline_open(ctx, hour_hand_path);

	// center axle fill
	graphics_context_set_fill_color(ctx, hour_hand_colour);
	graphics_fill_circle(ctx, center, axle_radius);

}

static void draw_baguette_hands(GContext *ctx) {
	int width = 7;

	// stroke by drawing line thicker
	graphics_context_set_stroke_width(ctx, width+2);
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	gpath_draw_outline_open(ctx, minute_hand_path);
	gpath_draw_outline_open(ctx, hour_hand_path);

	graphics_context_set_stroke_width(ctx, width);
	graphics_context_set_stroke_color(ctx, minute_hand_colour);
	gpath_draw_outline_open(ctx, minute_hand_path);
	graphics_context_set_stroke_color(ctx, hour_hand_colour);
	gpath_draw_outline_open(ctx, hour_hand_path);
}

static void draw_dauphine_hands(GContext *ctx) {
	graphics_context_set_fill_color(ctx, minute_hand_colour);
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	gpath_draw_outline(ctx, minute_hand_path);
	gpath_draw_filled(ctx, minute_hand_path);

	graphics_context_set_fill_color(ctx, hour_hand_colour);
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	gpath_draw_outline(ctx, hour_hand_path);
	gpath_draw_filled(ctx, hour_hand_path);
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
	switch (hand_shape) {
		case BAGUETTE: {
			draw_baguette_hands(ctx);
			break;
		}
		case PENCIL: {
			GRect bounds = layer_get_bounds(layer);
			draw_pencil_hands(grect_center_point(&bounds), ctx);
			break;
		}
		default:
		case DAUPHINE: {
			draw_dauphine_hands(ctx);
		}
	}
}

static void animation_teardown(Animation *animation) {
	init_animated = true;
}

static void animation_update(Animation *animation, const AnimationProgress progress) {
	int32_t total = ANIMATION_NORMALIZED_MAX - ANIMATION_NORMALIZED_MIN;
	int32_t hour_angle_start = calc_hour_angle(10,8);
	int32_t minute_angle_start = calc_minute_angle(8);

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
	hands_layer = layer;

	// set_hands_shape(PENCIL);
	set_hands_shape(DAUPHINE);
	// set_hands_shape(BAGUETTE);

	layer_set_update_proc(hands_layer, hands_update_proc);
}

void set_hour_hand_colour(GColor colour) {
	hour_hand_colour = colour;
}

void set_minute_hand_colour(GColor colour) {
	minute_hand_colour = colour;
}

void set_hands_shape(enum HandShape shape) {
	hand_shape = shape;

	set_hand_paths(hand_shape);
}

void set_hands(unsigned short hour, unsigned short minute) {
	hour_angle_target = calc_hour_angle(hour, minute);
	minute_angle_target = calc_minute_angle(minute);
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
