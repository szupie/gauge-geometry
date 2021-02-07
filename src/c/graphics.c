#include "graphics.h"
#include "big-digits.h"
#include "hands.h"
#include "weather.h"

#include "enamel.h"

static Layer* window_layer;

static TextLayer *day_text_layer, *date_text_layer, *day_shadow_text_layer, *date_shadow_text_layer_a, *date_shadow_text_layer_b;
static Layer *date_group_layer, *digits_layer, *ticks_canvas, *hands_layer, *temp_range_layer, *temp_now_layer;

static BatteryChargeState battery_state;

static int ticks_level;
static int charging_anim_frame = 0;

static GColor bg_colour;
static GColor date_colour;
static GColor ticks_colour;
static int ticks_size;

static bool battery_gauge_enabled = false;

static GPoint tick_positions[12];
static const int num_ticks = sizeof(tick_positions)/sizeof(tick_positions[0]);


static void init_tick_positions(Layer *layer) {
	GRect bounds = layer_get_bounds(layer);
	GRect insetRect = grect_inset(bounds, GEdgeInsets(RING_INSET));
	for (int i=0; i<num_ticks; i++) {
		int hour_angle = DEG_TO_TRIGANGLE(i*360/num_ticks);

		#if defined(PBL_ROUND)
		tick_positions[i] = gpoint_from_polar(insetRect, GOvalScaleModeFitCircle, hour_angle);
		#elif defined(PBL_RECT)
		tick_positions[i] = get_point_at_rect_perim(hour_angle, insetRect);
		#endif
	}
}

static void ticks_update_proc(Layer *layer, GContext *ctx) {
	graphics_context_set_stroke_width(ctx, 3);
	graphics_context_set_stroke_color(ctx, ticks_colour);

	bool should_animate = battery_gauge_enabled && battery_state.is_charging;
	// APP_LOG(APP_LOG_LEVEL_DEBUG, "redrawing ticks level %i, percentage %i", ticks_level, battery_state.charge_percent);

	for (int i=0; i<num_ticks; i++) {
		GPoint pos = tick_positions[i];
		bool should_fill = true;

		if (!should_animate) {
			should_fill = i < ticks_level;
		} else {
			// Charging animation
			// Single unfilled tick cycles bubbles up to current level
			should_fill = (i != charging_anim_frame) && (i < ticks_level);

			// Alt: ticks fill clockwise up to current level;
			// tick for current level always filled
			// should_fill = (i < charging_anim_frame) || (i == ticks_level-1);
		}
		// APP_LOG(APP_LOG_LEVEL_DEBUG, "should fill? %i, index %i", should_fill, i);

		if (should_fill) {
			graphics_context_set_fill_color(ctx, ticks_colour);
			graphics_fill_circle(ctx, pos, ticks_size);
		} else {
			if (ticks_size > 1) { // leave blank when tick size too small
				graphics_context_set_fill_color(ctx, bg_colour);
				int ring_size = ticks_size;
				if (ticks_size > 2) {
					ring_size = ticks_size-1;
				}
				graphics_draw_circle(ctx, pos, ring_size);
				graphics_fill_circle(ctx, pos, ring_size);
			}
		}
	}
}

static void init_text_style(TextLayer *layer) {
	GFont custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_POPPINS_16));
	text_layer_set_background_color(layer, GColorClear);
	text_layer_set_font(layer, custom_font);
	text_layer_set_text_alignment(layer, GTextAlignmentCenter);
	#if defined(PBL_RECT)
	text_layer_set_text_alignment(layer, GTextAlignmentLeft);
	#endif
}

// adjust date position to avoid overlapping with big digit for hour
static void update_date_group_position(unsigned short hour) {
	GRect frame = layer_get_frame(date_group_layer);

	#if defined(PBL_ROUND)
	GRect bounds = layer_get_bounds(window_layer);
	int center = bounds.size.w / 2;
	int left = center/2 - frame.size.w/2;

	if (hour >= 20) {
		frame.origin.x = left - 10;
	} else if (hour >= 10) {
		frame.origin.x = left - 8;
	} else {
		frame.origin.x = left;
	}
	#elif defined(PBL_RECT)
	if (hour >= 10) {
		frame.origin.x = 8;
	} else {
		frame.origin.x = 15;
	}
	#endif

	layer_set_frame(date_group_layer, frame);
}

void load_window(Window *window) {
	window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	// create day of week and date text layers
	int line_height = 16;
	int bottom = bounds.size.h / 2 - 3;
	int width = 80;
	#if defined(PBL_ROUND)
	int top = bottom - (line_height*2);
	#elif defined(PBL_RECT)
	int top = bottom - (line_height*3);
	#endif

	date_group_layer = layer_create(GRect(0, top, width, line_height*3.5));

	day_text_layer = text_layer_create(GRect(0, 0, width, line_height*1.5));
	day_shadow_text_layer = text_layer_create(GRect(TEXT_SHADOW_OFFSET, TEXT_SHADOW_OFFSET, width, line_height*1.5));

	date_text_layer = text_layer_create(GRect(0, line_height, width, line_height*2.5));
	date_shadow_text_layer_a = text_layer_create(GRect(TEXT_SHADOW_OFFSET, line_height + TEXT_SHADOW_OFFSET, width, line_height*2.5));
	date_shadow_text_layer_b = text_layer_create(GRect(TEXT_SHADOW_OFFSET, line_height - TEXT_SHADOW_OFFSET, width, line_height*2.5));

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
	init_tick_positions(ticks_canvas);
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

	switch (enamel_get_HANDS_SHAPE()) {
		case HANDS_SHAPE_BAGUETTE: {
			set_hands_shape(BAGUETTE);
			break;
		}
		case HANDS_SHAPE_PENCIL: {
			set_hands_shape(PENCIL);
			break;
		}
		default:
		case HANDS_SHAPE_DAUPHINE: {
			set_hands_shape(DAUPHINE);
			break;
		}
	}

	battery_gauge_enabled = enamel_get_BATTERY_GAUGE_ENABLED();
	enable_temp(enamel_get_TEMP_ENABLED());
	check_temp_unit_change();
	set_temp_range_colour(enamel_get_TEMP_RANGE_COLOUR());
	set_temp_now_colour(enamel_get_TEMP_NOW_COLOUR());
	
	layer_mark_dirty(hands_layer);
	update_battery_ticks(battery_state);

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

void update_battery_ticks(BatteryChargeState charge_state) {
	battery_state = charge_state;

	if (battery_gauge_enabled) {
		ticks_level = battery_state.charge_percent*0.01*num_ticks;
	} else {
		ticks_level = num_ticks;
	}

	layer_mark_dirty(ticks_canvas);
}

void animate_charging_indicator() {
	// increment tick animation
	charging_anim_frame = (charging_anim_frame+1)%ticks_level;
	layer_mark_dirty(ticks_canvas);
}

GColor get_bg_colour() {
	return bg_colour;
}

GPoint get_point_at_rect_perim(int angle, GRect frame) {
	// cornerAngle: angle between top right corner and top center
	// Get complement since atan2 is measured anticlockwise from +x axis, 
	// while pebble angles are measured clockwise from +y axis
	int32_t cornerAngle = TRIG_MAX_ANGLE/4 - atan2_lookup(frame.size.h, frame.size.w);
	angle = (angle + TRIG_MAX_ANGLE) % TRIG_MAX_ANGLE; // normalise angle
	int topRightAngle = cornerAngle;
	int topLeftAngle = TRIG_MAX_ANGLE - cornerAngle;
	int bottomRightAngle = TRIG_MAX_ANGLE/2 - cornerAngle;
	int bottomLeftAngle = TRIG_MAX_ANGLE/2 + cornerAngle;
	bool isTopEdge = angle < topRightAngle || angle > topLeftAngle;
	bool isBottomEdge = angle > bottomRightAngle && angle < bottomLeftAngle;

	GPoint center = GPoint(frame.origin.x + frame.size.w/2, frame.origin.y + frame.size.h/2);

	if (isTopEdge || isBottomEdge) {
		// top or bottom edges
		int8_t topBottomSign = cos_lookup(angle)/abs(cos_lookup(angle));
		int16_t yEdge = center.y - topBottomSign*frame.size.h/2;

		return GPoint(center.x + topBottomSign*(frame.size.h/2)*sin_lookup(angle)/cos_lookup(angle), yEdge);
	} else {
		// left or right edges
		int8_t leftRightSign = sin_lookup(angle)/abs(sin_lookup(angle));
		int16_t xEdge = center.x + leftRightSign*frame.size.w/2;

		return GPoint(xEdge, center.y - leftRightSign*(frame.size.w/2)*cos_lookup(angle)/sin_lookup(angle));
	}
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
