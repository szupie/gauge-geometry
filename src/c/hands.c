#include "hands.h"
#include "graphics.h"

static Layer *hands_layer;

static Animation *animation;
static AnimationImplementation animation_implementation;
static bool init_animated = false;
int32_t hour_angle_target, minute_angle_target;

enum HandShape hand_shape;
static GPath *minute_path, *hour_path;

static GPoint hour_start, hour_end, minute_start, minute_end, origin;
static int hour_length, minute_length;

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

// Get global position of point at y above origin, rotated to specified angle
static GPoint get_rotated_y_point(int32_t angle, int y, GPoint rot_origin) {
	return GPoint(
		rot_origin.x+sin_lookup(angle)*y/TRIG_MAX_RATIO, 
		rot_origin.y-cos_lookup(angle)*y/TRIG_MAX_RATIO
	);
}

static GPoint offset_point(GPoint point, GPoint offset) {
	return GPoint(
		point.x+offset.x,
		point.y+offset.y
	);
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
	int density = max(abs(diff.x), abs(diff.y)) * 4;

	graphics_context_set_fill_color(ctx, colour);
	for (int t=0; t<density; t++) {
		int x = start.x+t*diff.x/density;
		int y = start.y+t*diff.y/density;
		graphics_fill_circle(ctx, GPoint(x, y), thickness/2);
	}
}

// draw native stroke for path, or use custom fill for dithered grey for BW
static void draw_line_with_fallback(
	GContext *ctx,
	int thickness, GColor colour, 
	GPoint start, GPoint end) {
	graphics_context_set_stroke_width(ctx, thickness);
	graphics_context_set_stroke_color(ctx, colour);

	if (!should_dither_stroke(colour)) {
		graphics_draw_line(ctx, start, end);
	} else {
		fill_stroke(ctx, thickness, colour, start, end);
	}
}

static void update_hand_points() {
	int32_t hour_angle = hour_path->rotation;
	int32_t minute_angle = minute_path->rotation;

	hour_end = get_rotated_y_point(hour_angle, hour_length, origin);
	minute_end = get_rotated_y_point(minute_angle, minute_length, origin);

	if (hand_shape == SWISSRAIL) {
		hour_start = get_rotated_y_point(
			hour_angle, -SWISSRAIL_HOUR_EXTENDER_LENGTH, origin
		);
		minute_start = get_rotated_y_point(
			minute_angle, -SWISSRAIL_MINUTE_EXTENDER_LENGTH, origin
		);
	}
}


// Kickstarter style rounded hands
// This style is more abstract and less physical. The hands fuse together
// on the same layer, so the outlines go behind both hands
static void draw_baguette_hands(Layer *layer, GContext *ctx) {
	// outline by drawing line thicker
	graphics_context_set_stroke_width(ctx, BAGUETTE_THICKNESS+2);
	graphics_context_set_stroke_color(
		ctx, get_stroke_colour_for_fill(minute_hand_colour)
	);
	graphics_draw_line(ctx, origin, minute_end);
	graphics_context_set_stroke_color(
		ctx, get_stroke_colour_for_fill(hour_hand_colour)
	);
	graphics_draw_line(ctx, origin, hour_end);

	// actual hands
	draw_line_with_fallback(
		ctx,
		BAGUETTE_THICKNESS, minute_hand_colour, 
		origin, minute_end
	);
	draw_line_with_fallback(
		ctx,
		BAGUETTE_THICKNESS, hour_hand_colour, 
		origin, hour_end
	);
}

// Helper function to draw pencil hands at specified thickness and colour
// Draws unrounded shape with parallel sides at consistent thickness
// by repeatedly drawing same 1px path across transverse axis
// Point at center created by pushing outer lines away from rim towards center
static void draw_pencil_hand(
	GContext *ctx, int density, int32_t angle,
	GPoint start, GPoint end, 
	int thickness, GColor colour) {

	// move hand toward rim to prevent center end from jutting past center axle
	int outset_distance = thickness*1.5;

	float v_scale = v_component(angle);
	float h_scale = h_component(angle);

	// when drawing grey on BW screens, draw tip only to minimise operations
	int tip_length = outset_distance+thickness;
	GPoint tip_base = GPoint(
		end.x-v_scale*tip_length,
		end.y+h_scale*tip_length
	);

	graphics_context_set_stroke_width(ctx, 1);
	graphics_context_set_stroke_color(ctx, colour);

	for (int i=0; i<thickness*density; i++) {
		float transverse_offset = (i+1)/(float)density - thickness/2.0;
		float offcenterness = abs(i+1 - thickness*density/2.0)/(float)density;
		float pointy_offset = PENCIL_SHARPNESS*offcenterness - outset_distance;

		GPoint offset = GPoint(
			h_scale*transverse_offset - v_scale*pointy_offset,
			v_scale*transverse_offset + h_scale*pointy_offset
		);

		if (!should_dither_stroke(colour)) {
			graphics_draw_line(
				ctx,
				offset_point(start, offset),
				offset_point(end, offset)
			);
		} else {
			// fills are 2px wide, so this produces slightly thicker hands, 
			// but the change is sort of counterbalanced by dithering

			// Fill tip with lines radiating from one point (tip_base)
			// (for some reason there are weird (anti-aliasing?) artifacts 
			// when filling hand with parallel lines from origin to hand tip)
			GPoint tip = offset_point(end, offset);
			fill_stroke(ctx, 2, colour, tip_base, tip);
		}
	}
}

static void draw_pencil_hands(Layer *layer, GContext *ctx) {
	int density = 4;

	int outline_density = 2;
	int outline_outset = 2;

	int32_t hour_angle = hour_path->rotation;
	int32_t minute_angle = minute_path->rotation;

	GColor minute_stroke = get_stroke_colour_for_fill(minute_hand_colour);
	GColor hour_stroke = get_stroke_colour_for_fill(hour_hand_colour);

	GPoint minute_outline_offset = { 
		sin_lookup(minute_angle)*outline_outset/TRIG_MAX_RATIO, 
		-cos_lookup(minute_angle)*outline_outset/TRIG_MAX_RATIO
	};
	GPoint hour_outline_offset = { 
		sin_lookup(hour_angle)*outline_outset/TRIG_MAX_RATIO, 
		-cos_lookup(hour_angle)*outline_outset/TRIG_MAX_RATIO
	};

	GPoint minute_native_offset = {
		-sin_lookup(minute_angle)*PENCIL_THICKNESS/TRIG_MAX_RATIO, 
		cos_lookup(minute_angle)*PENCIL_THICKNESS/TRIG_MAX_RATIO
	};
	GPoint hour_native_offset = {
		-sin_lookup(hour_angle)*PENCIL_THICKNESS/TRIG_MAX_RATIO, 
		cos_lookup(hour_angle)*PENCIL_THICKNESS/TRIG_MAX_RATIO
	};

	// === Minute hand ===
	// minute hand outline (same path drawn thicker and outset) then fill
	graphics_context_set_stroke_width(ctx, PENCIL_THICKNESS+2);
	graphics_context_set_stroke_color(ctx, minute_stroke);
	graphics_draw_line(
		ctx,
		offset_point(origin, minute_native_offset),
		offset_point(minute_end, minute_native_offset)
	);
	draw_pencil_hand(
		ctx, outline_density, minute_angle,
		offset_point(origin, minute_outline_offset), 
		offset_point(minute_end, minute_outline_offset), 
		PENCIL_THICKNESS+2, minute_stroke
	);
	draw_pencil_hand(
		ctx, density, minute_angle,
		origin, minute_end,
		PENCIL_THICKNESS, minute_hand_colour
	);

	// draw native thick stroke to cover any missing areas
	// inset path to hide rounded ends
	draw_line_with_fallback(
		ctx,
		PENCIL_THICKNESS, minute_hand_colour, 
		offset_point(origin, minute_native_offset),
		offset_point(minute_end, minute_native_offset)
	);

	// === Hour hand ===
	// center axle outline
	graphics_context_set_stroke_width(ctx, 1);
	graphics_context_set_stroke_color(ctx, hour_stroke);
	graphics_draw_circle(ctx, origin, PENCIL_AXLE_RADIUS+1);

	// hour hand outline then fill
	graphics_context_set_stroke_width(ctx, PENCIL_THICKNESS+2);
	graphics_context_set_stroke_color(ctx, hour_stroke);
	graphics_draw_line(
		ctx,
		offset_point(origin, hour_native_offset),
		offset_point(hour_end, hour_native_offset)
	);
	draw_pencil_hand(
		ctx, outline_density, hour_angle,
		offset_point(origin, hour_outline_offset), 
		offset_point(hour_end, hour_outline_offset), 
		PENCIL_THICKNESS+2, hour_stroke
	);
	draw_pencil_hand(
		ctx, density, hour_angle,
		origin, hour_end,
		PENCIL_THICKNESS, hour_hand_colour
	);

	// native thick stroke for coverage
	draw_line_with_fallback(
		ctx,
		PENCIL_THICKNESS, hour_hand_colour, 
		offset_point(origin, hour_native_offset),
		offset_point(hour_end, hour_native_offset)
	);

	// center axle fill
	graphics_context_set_fill_color(ctx, hour_hand_colour);
	graphics_fill_circle(ctx, origin, PENCIL_AXLE_RADIUS);

}

static void draw_breguet_hands(Layer *layer, GContext *ctx) {
	int32_t hour_angle = hour_path->rotation;
	int32_t minute_angle = minute_path->rotation;

	int minute_hole_radius = BREGUET_MINUTE_RADIUS-BREGUET_EYE_THICKNESS;
	int hour_hole_radius = BREGUET_HOUR_RADIUS-BREGUET_EYE_THICKNESS;

	GColor minute_stroke = get_stroke_colour_for_fill(minute_hand_colour);
	GColor hour_stroke = get_stroke_colour_for_fill(hour_hand_colour);

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
	graphics_context_set_stroke_color(ctx, minute_stroke);
	graphics_context_set_fill_color(ctx, minute_stroke);

	// outline eye
	graphics_context_set_stroke_width(ctx, 1);
	graphics_fill_circle(ctx, minute_eye_pos, BREGUET_MINUTE_RADIUS+1);

	// outline hand by drawing line thicker
	graphics_context_set_stroke_width(ctx, BREGUET_THICKNESS+2);
	graphics_draw_line(ctx, origin, minute_end);

	// Fills
	graphics_context_set_fill_color(ctx, minute_hand_colour);

	// minute hand
	draw_line_with_fallback(
		ctx,
		BREGUET_THICKNESS, minute_hand_colour, 
		origin, minute_end
	);

	// minute hand eye (outside circle, then paint over "hole" with bg colour)
	graphics_fill_circle(ctx, minute_eye_pos, BREGUET_MINUTE_RADIUS);
	graphics_context_set_fill_color(ctx, get_bg_colour());
	graphics_context_set_stroke_color(ctx, minute_stroke);
	graphics_context_set_stroke_width(ctx, 1);
	graphics_fill_circle(ctx, minute_hole_pos, minute_hole_radius);
	graphics_draw_circle(ctx, minute_hole_pos, minute_hole_radius);

	// === Hour hand ===
	// Outlines
	graphics_context_set_stroke_color(ctx, hour_stroke);
	graphics_context_set_fill_color(ctx, hour_stroke);
	graphics_context_set_stroke_width(ctx, 1);

	// outline axle
	graphics_draw_circle(ctx, origin, BREGUET_AXLE_RADIUS+1);

	// outline eye
	graphics_fill_circle(ctx, hour_eye_pos, BREGUET_HOUR_RADIUS+1);

	// outline hand by drawing line thicker
	graphics_context_set_stroke_width(ctx, BREGUET_THICKNESS+2);
	graphics_draw_line(ctx, origin, hour_end);

	// Fills
	graphics_context_set_fill_color(ctx, hour_hand_colour);

	// center axle
	graphics_fill_circle(ctx, origin, BREGUET_AXLE_RADIUS);

	// hour hand
	draw_line_with_fallback(
		ctx,
		BREGUET_THICKNESS, hour_hand_colour, 
		origin, hour_end
	);

	// hour hand eye
	graphics_fill_circle(ctx, hour_eye_pos, BREGUET_HOUR_RADIUS);
	graphics_context_set_fill_color(ctx, get_bg_colour());
	graphics_context_set_stroke_color(ctx, hour_stroke);
	graphics_context_set_stroke_width(ctx, 1);
	graphics_fill_circle(ctx, hour_hole_pos, hour_hole_radius);
	graphics_draw_circle(ctx, hour_hole_pos, hour_hole_radius);
}

// Based on second hand of swiss railway station clocks
static void draw_swissrail_hands(Layer *layer, GContext *ctx) {
	// === Minute hand ===
	// Outlines
	graphics_context_set_stroke_color(
		ctx, get_stroke_colour_for_fill(minute_hand_colour)
	);
	graphics_context_set_stroke_width(ctx, 1);

	// outline bubble
	graphics_draw_circle(ctx, minute_end, SWISSRAIL_MINUTE_RADIUS+1);

	// outline by drawing line thicker
	graphics_context_set_stroke_width(ctx, SWISSRAIL_THICKNESS+2);
	// gpath_draw_outline_open(ctx, minute_path);
	graphics_draw_line(ctx, minute_start, minute_end);

	// minute hand
	graphics_context_set_stroke_width(ctx, SWISSRAIL_THICKNESS);
	graphics_context_set_fill_color(ctx, minute_hand_colour);
	graphics_fill_circle(ctx, minute_end, SWISSRAIL_MINUTE_RADIUS);
	draw_line_with_fallback(
		ctx,
		SWISSRAIL_THICKNESS, minute_hand_colour, 
		minute_start, minute_end
	);

	// === Hour hand ===
	// Outlines
	graphics_context_set_stroke_color(
		ctx, get_stroke_colour_for_fill(hour_hand_colour)
	);
	graphics_context_set_stroke_width(ctx, 1);

	// outline axle
	graphics_draw_circle(ctx, origin, SWISSRAIL_AXLE_RADIUS+1);

	// outline bubble
	graphics_draw_circle(ctx, hour_end, SWISSRAIL_HOUR_RADIUS+1);

	// outline by drawing line thicker
	graphics_context_set_stroke_width(ctx, SWISSRAIL_THICKNESS+2);
	// gpath_draw_outline_open(ctx, hour_path);
	graphics_draw_line(ctx, hour_start, hour_end);

	// hour hand
	graphics_context_set_stroke_width(ctx, SWISSRAIL_THICKNESS);
	graphics_context_set_fill_color(ctx, hour_hand_colour);
	graphics_fill_circle(ctx, hour_end, SWISSRAIL_HOUR_RADIUS);
	draw_line_with_fallback(
		ctx,
		SWISSRAIL_THICKNESS, hour_hand_colour, 
		hour_start, hour_end
	);

	// center axle fill
	graphics_fill_circle(ctx, origin, SWISSRAIL_AXLE_RADIUS);
}

static void draw_dauphine_hands(Layer *layer, GContext *ctx) {
	graphics_context_set_fill_color(ctx, minute_hand_colour);
	graphics_context_set_stroke_color(
		ctx, get_stroke_colour_for_fill(minute_hand_colour)
	);
	gpath_draw_filled(ctx, minute_path);
	gpath_draw_outline(ctx, minute_path);

	graphics_context_set_fill_color(ctx, hour_hand_colour);
	graphics_context_set_stroke_color(
		ctx, get_stroke_colour_for_fill(hour_hand_colour)
	);
	gpath_draw_filled(ctx, hour_path);
	gpath_draw_outline(ctx, hour_path);
}

static void set_hands_shape(enum HandShape shape) {
	hand_shape = shape;
	
	switch (hand_shape) {
		case BAGUETTE: {
			hour_length = BAGUETTE_HOUR_LENGTH;
			minute_length = BAGUETTE_MINUTE_LENGTH;
			layer_set_update_proc(hands_layer, draw_baguette_hands);
			break;
		}
		case PENCIL: {
			hour_length = PENCIL_HOUR_LENGTH;
			minute_length = PENCIL_MINUTE_LENGTH;
			layer_set_update_proc(hands_layer, draw_pencil_hands);
			break;
		}
		case BREGUET: {
			hour_length = BREGUET_HOUR_LENGTH;
			minute_length = BREGUET_MINUTE_LENGTH;
			layer_set_update_proc(hands_layer, draw_breguet_hands);
			break;
		}
		case SWISSRAIL: {
			hour_length = SWISSRAIL_HOUR_LENGTH;
			minute_length = SWISSRAIL_MINUTE_LENGTH;
			layer_set_update_proc(hands_layer, draw_swissrail_hands);
			break;
		}
		default:
		case DAUPHINE: {
			layer_set_update_proc(hands_layer, draw_dauphine_hands);
		}
	}
	update_hand_points();
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
	update_hand_points();
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

	hour_path = gpath_create(&DAUPHINE_HOUR_POINTS);
	minute_path = gpath_create(&DAUPHINE_MINUTE_POINTS);

	GRect bounds = layer_get_bounds(hands_layer);
	origin = grect_center_point(&bounds);
	gpath_move_to(hour_path, origin);
	gpath_move_to(minute_path, origin);

	set_hands_shape(DAUPHINE);
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
		update_hand_points();
		layer_mark_dirty(hands_layer);
	} else {
		start_animation();
	}
}

void destroy_hands() {
	gpath_destroy(hour_path);
	gpath_destroy(minute_path);
}
