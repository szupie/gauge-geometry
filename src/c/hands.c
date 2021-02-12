#include "graphics.h"
#include "hands.h"

static Layer *hands_layer;

static Animation *animation;
static AnimationImplementation animation_implementation;
static bool init_animated = false;
int32_t hour_angle_target, minute_angle_target;

enum HandShape hand_shape;
static GPath *minute_path, *hour_path;

static GColor hour_hand_colour;
static GColor minute_hand_colour;

static int32_t calc_hour_angle(unsigned short hour, unsigned short minute) {
	return TRIG_MAX_ANGLE * ((hour % 12) + (minute / 60.0)) / 12.0;
}

static int32_t calc_minute_angle(unsigned short minute) {
	return TRIG_MAX_ANGLE * minute / 60.0;
}

static float v_component(int32_t angle) {
	return sin_lookup(angle)/(float)TRIG_MAX_RATIO;
}

static float h_component(int32_t angle) {
	return cos_lookup(angle)/(float)TRIG_MAX_RATIO;
}

static void create_hand_paths(enum HandShape shape) {
	gpath_destroy(hour_path);
	gpath_destroy(minute_path);

	switch (shape) {
		case BAGUETTE: {
			hour_path = gpath_create(&BAGUETTE_HOUR_POINTS);
			minute_path = gpath_create(&BAGUETTE_MINUTE_POINTS);
			break;
		}
		case PENCIL: {
			hour_path = gpath_create(&PENCIL_HOUR_POINTS);
			minute_path = gpath_create(&PENCIL_MINUTE_POINTS);
			break;
		}
		case BREGUET: {
			hour_path = gpath_create(&BREGUET_HOUR_POINTS);
			minute_path = gpath_create(&BREGUET_MINUTE_POINTS);
			break;
		}
		case SWISSRAIL: {
			hour_path = gpath_create(&SWISSRAIL_HOUR_POINTS);
			minute_path = gpath_create(&SWISSRAIL_MINUTE_POINTS);
			break;
		}
		default:
		case DAUPHINE: {
			hour_path = gpath_create(&DAUPHINE_HOUR_POINTS);
			minute_path = gpath_create(&DAUPHINE_MINUTE_POINTS);
			break;
		}
	}
}

// Kickstarter style rounded hands
// This style is more abstract and less physical. The hands fuse together
// on the same layer, so the outlines go behind both hands
static void draw_baguette_hands(GContext *ctx) {
	// outline by drawing line thicker
	graphics_context_set_stroke_width(ctx, BAGUETTE_THICKNESS+2);
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	gpath_draw_outline_open(ctx, minute_path);
	gpath_draw_outline_open(ctx, hour_path);

	graphics_context_set_stroke_width(ctx, BAGUETTE_THICKNESS);
	graphics_context_set_stroke_color(ctx, minute_hand_colour);
	gpath_draw_outline_open(ctx, minute_path);
	graphics_context_set_stroke_color(ctx, hour_hand_colour);
	gpath_draw_outline_open(ctx, hour_path);
}

// Helper function to draw pencil hands at specified thickness and colour
// Draws unrounded shape with parallel sides at consistent thickness
// by repeatedly drawing same 1px path across transverse axis
// Point at center created by pushing inner lines outwards from center
static void draw_pencil_hand(GPath * path, int width, int density, GColor colour, GPoint origin, GContext *ctx) {
	// move hand toward rim to prevent center end from jutting past center axle
	int outset_distance = width*1.5;

	int32_t angle = path->rotation;
	float v_scale = v_component(angle);
	float h_scale = h_component(angle);

	graphics_context_set_stroke_width(ctx, 1);
	graphics_context_set_stroke_color(ctx, colour);

	for (int i=0; i<width*density; i++) {
		float side_offset = (i+1)/(float)density - width/2.0;
		float offcenterness = abs((i+1) - width*density/2.0)/(float)density;
		float pointy_offset = PENCIL_SHARPNESS*offcenterness - outset_distance;

		float offset_x = h_scale*side_offset - v_scale*pointy_offset;
		float offset_y = v_scale*side_offset + h_scale*pointy_offset;

		gpath_move_to(path, GPoint(origin.x+offset_x, origin.y+offset_y));
		gpath_draw_outline_open(ctx, path);
	}
}

static void draw_pencil_hands(GPoint center, GContext *ctx) {
	int density = 4;

	int stroke_density = 2;
	int stroke_outset = 2;

	int32_t hour_angle = hour_path->rotation;
	int32_t minute_angle = minute_path->rotation;

	// === Minute hand ===
	// minute hand stroke (same path drawn thicker and outset) then fill
	GPoint minute_stroke_origin = { 
		center.x+sin_lookup(minute_angle)*stroke_outset/TRIG_MAX_RATIO, 
		center.y-cos_lookup(minute_angle)*stroke_outset/TRIG_MAX_RATIO
	};
	draw_pencil_hand(minute_path, PENCIL_THICKNESS+2, stroke_density, get_bg_colour(), minute_stroke_origin, ctx);
	draw_pencil_hand(minute_path, PENCIL_THICKNESS, density, minute_hand_colour, center, ctx);

	// draw native thick stroke to cover any missing areas
	// inset path to hide rounded ends
	GPoint minute_native_origin = {
		center.x-sin_lookup(minute_angle)*PENCIL_THICKNESS/TRIG_MAX_RATIO, 
		center.y+cos_lookup(minute_angle)*PENCIL_THICKNESS/TRIG_MAX_RATIO
	};
	gpath_move_to(minute_path, minute_native_origin);
	graphics_context_set_stroke_width(ctx, PENCIL_THICKNESS);
	graphics_context_set_stroke_color(ctx, minute_hand_colour);
	gpath_draw_outline_open(ctx, minute_path);


	// === Hour hand ===
	// center axle stroke
	graphics_context_set_stroke_width(ctx, 1);
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	graphics_draw_circle(ctx, center, PENCIL_AXLE_RADIUS+1);

	// hour hand stroke then fill
	GPoint hour_stroke_origin = { 
		center.x+sin_lookup(hour_angle)*stroke_outset/TRIG_MAX_RATIO, 
		center.y-cos_lookup(hour_angle)*stroke_outset/TRIG_MAX_RATIO
	};
	draw_pencil_hand(hour_path, PENCIL_THICKNESS+2, stroke_density, get_bg_colour(), hour_stroke_origin, ctx);
	draw_pencil_hand(hour_path, PENCIL_THICKNESS, density, hour_hand_colour, center, ctx);

	// native thick stroke for coverage
	GPoint hour_native_origin = {
		center.x-sin_lookup(hour_angle)*PENCIL_THICKNESS/TRIG_MAX_RATIO, 
		center.y+cos_lookup(hour_angle)*PENCIL_THICKNESS/TRIG_MAX_RATIO
	};
	gpath_move_to(hour_path, hour_native_origin);
	graphics_context_set_stroke_width(ctx, PENCIL_THICKNESS);
	graphics_context_set_stroke_color(ctx, hour_hand_colour);
	gpath_draw_outline_open(ctx, hour_path);

	// center axle fill
	graphics_context_set_fill_color(ctx, hour_hand_colour);
	graphics_fill_circle(ctx, center, PENCIL_AXLE_RADIUS);

}

static void draw_breguet_hands(GPoint center, GContext *ctx) {
	int32_t hour_angle = hour_path->rotation;
	int32_t minute_angle = minute_path->rotation;

	const int minute_hole_radius = BREGUET_MINUTE_RADIUS-BREGUET_EYE_THICKNESS;
	const int hour_hole_radius = BREGUET_HOUR_RADIUS-BREGUET_EYE_THICKNESS;

	GPoint hour_eye_pos = {
		center.x+v_component(hour_angle)*BREGUET_HOUR_EYE_DIST, 
		center.y-h_component(hour_angle)*BREGUET_HOUR_EYE_DIST
	};
	GPoint hour_hole_pos = {
		hour_eye_pos.x+v_component(hour_angle)*(BREGUET_EYE_THICKNESS-1), 
		hour_eye_pos.y-h_component(hour_angle)*(BREGUET_EYE_THICKNESS-1)
	};
	GPoint minute_eye_pos = {
		center.x+v_component(minute_angle)*BREGUET_MINUTE_EYE_DIST, 
		center.y-h_component(minute_angle)*BREGUET_MINUTE_EYE_DIST
	};
	GPoint minute_hole_pos = {
		minute_eye_pos.x+v_component(minute_angle)*(BREGUET_EYE_THICKNESS-1), 
		minute_eye_pos.y-h_component(minute_angle)*(BREGUET_EYE_THICKNESS-1)
	};

	// === Minute hand ===
	// Outlines
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	graphics_context_set_fill_color(ctx, get_bg_colour());

	// outline eye
	graphics_context_set_stroke_width(ctx, 1);
	graphics_fill_circle(ctx, minute_eye_pos, BREGUET_MINUTE_RADIUS+1);

	// outline hand by drawing line thicker
	graphics_context_set_stroke_width(ctx, BREGUET_THICKNESS+2);
	gpath_draw_outline_open(ctx, minute_path);

	// Fills
	graphics_context_set_stroke_color(ctx, minute_hand_colour);
	graphics_context_set_fill_color(ctx, minute_hand_colour);

	// minute hand
	graphics_context_set_stroke_width(ctx, BREGUET_THICKNESS);
	gpath_draw_outline_open(ctx, minute_path);

	// minute hand eye
	graphics_fill_circle(ctx, minute_eye_pos, BREGUET_MINUTE_RADIUS);
	graphics_context_set_fill_color(ctx, get_bg_colour());
	graphics_fill_circle(ctx, minute_hole_pos, minute_hole_radius);

	// === Hour hand ===
	// Outlines
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	graphics_context_set_fill_color(ctx, get_bg_colour());
	graphics_context_set_stroke_width(ctx, 1);

	// outline axle
	graphics_draw_circle(ctx, center, BREGUET_AXLE_RADIUS+1);

	// outline eye
	graphics_fill_circle(ctx, hour_eye_pos, BREGUET_HOUR_RADIUS+1);

	// outline hand by drawing line thicker
	graphics_context_set_stroke_width(ctx, BREGUET_THICKNESS+2);
	gpath_draw_outline_open(ctx, hour_path);

	// Fills
	graphics_context_set_stroke_color(ctx, hour_hand_colour);
	graphics_context_set_fill_color(ctx, hour_hand_colour);

	// center axle
	graphics_fill_circle(ctx, center, BREGUET_AXLE_RADIUS);

	// hour hand
	graphics_context_set_stroke_width(ctx, BREGUET_THICKNESS);
	gpath_draw_outline_open(ctx, hour_path);

	// hour hand eye
	graphics_fill_circle(ctx, hour_eye_pos, BREGUET_HOUR_RADIUS);
	graphics_context_set_fill_color(ctx, get_bg_colour());
	graphics_fill_circle(ctx, hour_hole_pos, hour_hole_radius);
}

// Based on second hand of swiss railway station clocks
static void draw_swissrail_hands(GPoint center, GContext *ctx) {
	int hour_bubble_dist = -hour_path->points[hour_path->num_points-1].y;
	int minute_bubble_dist = -minute_path->points[minute_path->num_points-1].y;

	int32_t hour_angle = hour_path->rotation;
	int32_t minute_angle = minute_path->rotation;

	GPoint hour_bubble_pos = {
		center.x+sin_lookup(hour_angle)*hour_bubble_dist/TRIG_MAX_RATIO, 
		center.y-cos_lookup(hour_angle)*hour_bubble_dist/TRIG_MAX_RATIO
	};
	GPoint minute_bubble_pos = {
		center.x+sin_lookup(minute_angle)*minute_bubble_dist/TRIG_MAX_RATIO, 
		center.y-cos_lookup(minute_angle)*minute_bubble_dist/TRIG_MAX_RATIO
	};

	// === Minute hand ===
	// Outlines
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	graphics_context_set_stroke_width(ctx, 1);

	// outline bubble
	graphics_draw_circle(ctx, minute_bubble_pos, SWISSRAIL_MINUTE_RADIUS+1);

	// outline by drawing line thicker
	graphics_context_set_stroke_width(ctx, SWISSRAIL_THICKNESS+2);
	gpath_draw_outline_open(ctx, minute_path);

	// minute hand
	graphics_context_set_stroke_width(ctx, SWISSRAIL_THICKNESS);
	graphics_context_set_stroke_color(ctx, minute_hand_colour);
	graphics_context_set_fill_color(ctx, minute_hand_colour);
	gpath_draw_outline_open(ctx, minute_path);
	graphics_fill_circle(ctx, minute_bubble_pos, SWISSRAIL_MINUTE_RADIUS);

	// === Hour hand ===
	// Outlines
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	graphics_context_set_stroke_width(ctx, 1);

	// outline axle
	graphics_draw_circle(ctx, center, SWISSRAIL_AXLE_RADIUS+1);

	// outline bubble
	graphics_draw_circle(ctx, hour_bubble_pos, SWISSRAIL_HOUR_RADIUS+1);

	// outline by drawing line thicker
	graphics_context_set_stroke_width(ctx, SWISSRAIL_THICKNESS+2);
	gpath_draw_outline_open(ctx, hour_path);

	// hour hand
	graphics_context_set_stroke_width(ctx, SWISSRAIL_THICKNESS);
	graphics_context_set_stroke_color(ctx, hour_hand_colour);
	graphics_context_set_fill_color(ctx, hour_hand_colour);
	gpath_draw_outline_open(ctx, hour_path);
	graphics_fill_circle(ctx, hour_bubble_pos, SWISSRAIL_HOUR_RADIUS);

	// center axle fill
	graphics_fill_circle(ctx, center, SWISSRAIL_AXLE_RADIUS);
}


static void draw_dauphine_hands(GContext *ctx) {
	graphics_context_set_fill_color(ctx, minute_hand_colour);
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	gpath_draw_outline(ctx, minute_path);
	gpath_draw_filled(ctx, minute_path);

	graphics_context_set_fill_color(ctx, hour_hand_colour);
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	gpath_draw_outline(ctx, hour_path);
	gpath_draw_filled(ctx, hour_path);
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
		case BREGUET: {
			GRect bounds = layer_get_bounds(layer);
			draw_breguet_hands(grect_center_point(&bounds), ctx);
			break;
		}
		case SWISSRAIL: {
			GRect bounds = layer_get_bounds(layer);
			draw_swissrail_hands(grect_center_point(&bounds), ctx);
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

	gpath_rotate_to(hour_path, hour_angle);
	gpath_rotate_to(minute_path, minute_angle);
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

	set_hands_shape(DAUPHINE);

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

	create_hand_paths(hand_shape);

	GRect bounds = layer_get_bounds(hands_layer);
	gpath_move_to(hour_path, grect_center_point(&bounds));
	gpath_move_to(minute_path, grect_center_point(&bounds));

	gpath_rotate_to(hour_path, hour_angle_target);
	gpath_rotate_to(minute_path, minute_angle_target);
}

void set_hands(unsigned short hour, unsigned short minute) {
	hour_angle_target = calc_hour_angle(hour, minute);
	minute_angle_target = calc_minute_angle(minute);
	// APP_LOG(APP_LOG_LEVEL_DEBUG, "setting hands %d, %d", hour, minute);

	if (init_animated) {
		gpath_rotate_to(hour_path, hour_angle_target);
		gpath_rotate_to(minute_path, minute_angle_target);
		layer_mark_dirty(hands_layer);
	} else {
		start_animation();
	}
}

void destroy_hands() {
	gpath_destroy(hour_path);
	gpath_destroy(minute_path);
}
