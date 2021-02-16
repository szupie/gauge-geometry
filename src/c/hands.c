#include "hands.h"
#include "graphics.h"

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

// Get global position of hand path point at index n with current rotation
static GPoint get_point_n(GPath * path, int n, GPoint origin) {
	int distance = -path->points[n].y;
	int32_t angle = path->rotation;

	return GPoint(
		origin.x+sin_lookup(angle)*distance/TRIG_MAX_RATIO, 
		origin.y-cos_lookup(angle)*distance/TRIG_MAX_RATIO
	);
}

static GPoint get_end_point(GPath * path, GPoint origin) {
	return get_point_n(path, path->num_points-1, origin);
}

static int max(int a, int b) {
    return a > b ? a : b;
}

// On BW displays, grey strokes need custom dithering via fills
static bool should_dither_stroke(GColor colour) {
	return PBL_IF_BW_ELSE(gcolor_equal(colour, GColorLightGray), false);
}

// Draw line using filled circles to emulate dithered grey stroke
static void fill_stroke(
	GContext *ctx, 
	int thickness, GColor colour, 
	GPoint start, GPoint end) {

	GPoint diff = GPoint(end.x-start.x, end.y-start.y);
	int steps = max(abs(diff.x), abs(diff.y));

	graphics_context_set_fill_color(ctx, colour);
	for (int t=0; t<steps; t++) {
		int x = start.x+t*diff.x/steps;
		int y = start.y+t*diff.y/steps;
		graphics_fill_circle(ctx, GPoint(x, y), thickness/2);
	}
}

// draw native stroke for path, or use custom fill for dithered grey for BW
static void stroke_path_with_fallback(
	GContext *ctx, GPath *path, 
	int thickness, GColor colour, 
	GPoint start, GPoint end) {

	if (!should_dither_stroke(colour)) {
		graphics_context_set_stroke_color(ctx, colour);
		gpath_draw_outline_open(ctx, path);
	} else {
		fill_stroke(ctx, thickness, colour, start, end);
	}
}

// Kickstarter style rounded hands
// This style is more abstract and less physical. The hands fuse together
// on the same layer, so the outlines go behind both hands
static void draw_baguette_hands(GPoint origin, GContext *ctx) {
	// outline by drawing line thicker
	graphics_context_set_stroke_width(ctx, BAGUETTE_THICKNESS+2);
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	gpath_draw_outline_open(ctx, minute_path);
	gpath_draw_outline_open(ctx, hour_path);

	graphics_context_set_stroke_width(ctx, BAGUETTE_THICKNESS);

	stroke_path_with_fallback(
		ctx, minute_path, 
		BAGUETTE_THICKNESS, minute_hand_colour, 
		origin, get_end_point(minute_path, origin)
	);

	stroke_path_with_fallback(
		ctx, hour_path, 
		BAGUETTE_THICKNESS, hour_hand_colour, 
		origin, get_end_point(hour_path, origin)
	);
}

// Helper function to draw pencil hands at specified thickness and colour
// Draws unrounded shape with parallel sides at consistent thickness
// by repeatedly drawing same 1px path across transverse axis
// Point at center created by pushing outer lines away from rim towards center
static void draw_pencil_hand(
	GContext *ctx, GPath * path, GPoint origin, 
	int thickness, int density, GColor colour) {

	// move hand toward rim to prevent center end from jutting past center axle
	int outset_distance = thickness*1.5;

	int32_t angle = path->rotation;
	float v_scale = v_component(angle);
	float h_scale = h_component(angle);

	// when drawing grey on BW screens, draw tip only to minimise operations
	int tip_length = outset_distance+thickness;
	float tip_base_offset_x = -v_scale*tip_length;
	float tip_base_offset_y = h_scale*tip_length;

	graphics_context_set_stroke_width(ctx, 1);
	graphics_context_set_stroke_color(ctx, colour);

	for (int i=0; i<thickness*density; i++) {
		float transverse_offset = (i+1)/(float)density - thickness/2.0;
		float offcenterness = abs(i+1 - thickness*density/2.0)/(float)density;
		float pointy_offset = PENCIL_SHARPNESS*offcenterness - outset_distance;

		float offset_x = h_scale*transverse_offset - v_scale*pointy_offset;
		float offset_y = v_scale*transverse_offset + h_scale*pointy_offset;

		if (!should_dither_stroke(colour)) {
			gpath_move_to(path, GPoint(origin.x+offset_x, origin.y+offset_y));
			gpath_draw_outline_open(ctx, path);
		} else {
			// fills are 2px wide, so this produces slightly thicker hands, 
			// but the change is sort of counterbalanced by dithering
			GPoint path_end = get_end_point(path, origin);

			// Fill tip with lines radiating from one point
			// (for some reason there are weird (anti-aliasing?) artifacts 
			// when filling hand with parallel lines from origin to hand tip)
			int tip_base_x = path_end.x+tip_base_offset_x;
			int tip_base_y = path_end.y+tip_base_offset_y;

			GPoint start = GPoint(tip_base_x, tip_base_y);
			GPoint end = GPoint(path_end.x+offset_x, path_end.y+offset_y);
			fill_stroke(ctx, 2, colour, start, end);
		}
	}
}

static void draw_pencil_hands(GPoint origin, GContext *ctx) {
	int density = 4;

	int stroke_density = 2;
	int stroke_outset = 2;

	int32_t hour_angle = hour_path->rotation;
	int32_t minute_angle = minute_path->rotation;

	// === Minute hand ===
	// minute hand outline (same path drawn thicker and outset) then fill
	GPoint minute_outline_origin = { 
		origin.x+sin_lookup(minute_angle)*stroke_outset/TRIG_MAX_RATIO, 
		origin.y-cos_lookup(minute_angle)*stroke_outset/TRIG_MAX_RATIO
	};
	draw_pencil_hand(
		ctx, minute_path, minute_outline_origin, 
		PENCIL_THICKNESS+2, stroke_density, get_bg_colour()
	);
	draw_pencil_hand(
		ctx, minute_path, origin, 
		PENCIL_THICKNESS, density, minute_hand_colour
	);

	// draw native thick stroke to cover any missing areas
	// inset path to hide rounded ends
	GPoint minute_thick_origin = {
		origin.x-sin_lookup(minute_angle)*PENCIL_THICKNESS/TRIG_MAX_RATIO, 
		origin.y+cos_lookup(minute_angle)*PENCIL_THICKNESS/TRIG_MAX_RATIO
	};
	gpath_move_to(minute_path, minute_thick_origin);
	graphics_context_set_stroke_width(ctx, PENCIL_THICKNESS);
	// dithered hand needs to be drawn wider to reduce jagged effect
	stroke_path_with_fallback(
		ctx, minute_path, 
		PENCIL_THICKNESS+1, minute_hand_colour, 
		minute_thick_origin, get_end_point(minute_path, minute_thick_origin)
	);


	// === Hour hand ===
	// center axle stroke
	graphics_context_set_stroke_width(ctx, 1);
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	graphics_draw_circle(ctx, origin, PENCIL_AXLE_RADIUS+1);

	// hour hand outline then fill
	GPoint hour_outline_origin = { 
		origin.x+sin_lookup(hour_angle)*stroke_outset/TRIG_MAX_RATIO, 
		origin.y-cos_lookup(hour_angle)*stroke_outset/TRIG_MAX_RATIO
	};
	draw_pencil_hand(
		ctx, hour_path, hour_outline_origin, 
		PENCIL_THICKNESS+2, stroke_density, get_bg_colour()
	);
	draw_pencil_hand(
		ctx, hour_path, origin, 
		PENCIL_THICKNESS, density, hour_hand_colour
	);

	// native thick stroke for coverage
	GPoint hour_thick_origin = {
		origin.x-sin_lookup(hour_angle)*PENCIL_THICKNESS/TRIG_MAX_RATIO, 
		origin.y+cos_lookup(hour_angle)*PENCIL_THICKNESS/TRIG_MAX_RATIO
	};
	gpath_move_to(hour_path, hour_thick_origin);
	graphics_context_set_stroke_width(ctx, PENCIL_THICKNESS);
	stroke_path_with_fallback(
		ctx, hour_path, 
		PENCIL_THICKNESS+1, hour_hand_colour, 
		hour_thick_origin, get_end_point(hour_path, hour_thick_origin)
	);

	// center axle fill
	graphics_context_set_fill_color(ctx, hour_hand_colour);
	graphics_fill_circle(ctx, origin, PENCIL_AXLE_RADIUS);

}

static void draw_breguet_hands(GPoint origin, GContext *ctx) {
	int32_t hour_angle = hour_path->rotation;
	int32_t minute_angle = minute_path->rotation;

	const int minute_hole_radius = BREGUET_MINUTE_RADIUS-BREGUET_EYE_THICKNESS;
	const int hour_hole_radius = BREGUET_HOUR_RADIUS-BREGUET_EYE_THICKNESS;

	GPoint hour_eye_pos = {
		origin.x+v_component(hour_angle)*BREGUET_HOUR_EYE_DIST, 
		origin.y-h_component(hour_angle)*BREGUET_HOUR_EYE_DIST
	};
	GPoint hour_hole_pos = {
		hour_eye_pos.x+v_component(hour_angle)*(BREGUET_EYE_THICKNESS-1), 
		hour_eye_pos.y-h_component(hour_angle)*(BREGUET_EYE_THICKNESS-1)
	};
	GPoint minute_eye_pos = {
		origin.x+v_component(minute_angle)*BREGUET_MINUTE_EYE_DIST, 
		origin.y-h_component(minute_angle)*BREGUET_MINUTE_EYE_DIST
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
	graphics_context_set_fill_color(ctx, minute_hand_colour);

	// minute hand
	graphics_context_set_stroke_width(ctx, BREGUET_THICKNESS);
	gpath_draw_outline_open(ctx, minute_path);
	stroke_path_with_fallback(
		ctx, minute_path, 
		BREGUET_THICKNESS, minute_hand_colour, 
		origin, get_end_point(minute_path, origin)
	);

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
	graphics_draw_circle(ctx, origin, BREGUET_AXLE_RADIUS+1);

	// outline eye
	graphics_fill_circle(ctx, hour_eye_pos, BREGUET_HOUR_RADIUS+1);

	// outline hand by drawing line thicker
	graphics_context_set_stroke_width(ctx, BREGUET_THICKNESS+2);
	gpath_draw_outline_open(ctx, hour_path);

	// Fills
	graphics_context_set_fill_color(ctx, hour_hand_colour);

	// center axle
	graphics_fill_circle(ctx, origin, BREGUET_AXLE_RADIUS);

	// hour hand
	graphics_context_set_stroke_width(ctx, BREGUET_THICKNESS);
	stroke_path_with_fallback(
		ctx, hour_path, 
		BREGUET_THICKNESS, hour_hand_colour, 
		origin, get_end_point(hour_path, origin)
	);

	// hour hand eye
	graphics_fill_circle(ctx, hour_eye_pos, BREGUET_HOUR_RADIUS);
	graphics_context_set_fill_color(ctx, get_bg_colour());
	graphics_fill_circle(ctx, hour_hole_pos, hour_hole_radius);
}

// Based on second hand of swiss railway station clocks
static void draw_swissrail_hands(GPoint origin, GContext *ctx) {
	GPoint hour_bubble_pos = get_end_point(hour_path, origin);
	GPoint minute_bubble_pos = get_end_point(minute_path, origin);

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
	graphics_context_set_fill_color(ctx, minute_hand_colour);
	graphics_fill_circle(ctx, minute_bubble_pos, SWISSRAIL_MINUTE_RADIUS);
	stroke_path_with_fallback(
		ctx, minute_path, 
		SWISSRAIL_THICKNESS, minute_hand_colour, 
		get_point_n(minute_path, 0, origin), get_end_point(minute_path, origin)
	);

	// === Hour hand ===
	// Outlines
	graphics_context_set_stroke_color(ctx, get_bg_colour());
	graphics_context_set_stroke_width(ctx, 1);

	// outline axle
	graphics_draw_circle(ctx, origin, SWISSRAIL_AXLE_RADIUS+1);

	// outline bubble
	graphics_draw_circle(ctx, hour_bubble_pos, SWISSRAIL_HOUR_RADIUS+1);

	// outline by drawing line thicker
	graphics_context_set_stroke_width(ctx, SWISSRAIL_THICKNESS+2);
	gpath_draw_outline_open(ctx, hour_path);

	// hour hand
	graphics_context_set_stroke_width(ctx, SWISSRAIL_THICKNESS);
	graphics_context_set_fill_color(ctx, hour_hand_colour);
	graphics_fill_circle(ctx, hour_bubble_pos, SWISSRAIL_HOUR_RADIUS);
	stroke_path_with_fallback(
		ctx, hour_path, 
		SWISSRAIL_THICKNESS, hour_hand_colour, 
		get_point_n(hour_path, 0, origin), get_end_point(hour_path, origin)
	);

	// center axle fill
	graphics_fill_circle(ctx, origin, SWISSRAIL_AXLE_RADIUS);
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
			GRect bounds = layer_get_bounds(layer);
			draw_baguette_hands(grect_center_point(&bounds), ctx);
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

static void create_hand_paths(enum HandShape shape) {
	destroy_hands();

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

static void set_hands_shape(enum HandShape shape) {
	hand_shape = shape;

	create_hand_paths(hand_shape);

	GRect bounds = layer_get_bounds(hands_layer);
	gpath_move_to(hour_path, grect_center_point(&bounds));
	gpath_move_to(minute_path, grect_center_point(&bounds));

	gpath_rotate_to(hour_path, hour_angle_target);
	gpath_rotate_to(minute_path, minute_angle_target);
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

void update_hands_settings(GColor hour, GColor minute, enum HandShape shape) {
	hour_hand_colour = hour;
	minute_hand_colour = minute;
	set_hands_shape(shape);

	layer_mark_dirty(hands_layer);
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
