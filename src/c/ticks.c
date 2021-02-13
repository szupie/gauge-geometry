#include "ticks.h"
#include "graphics.h"

static GColor ticks_colour;
static int ticks_size;

#define num_ticks 12
static GPoint tick_positions[num_ticks];
static Layer *ticks_layer;

static int ticks_level;
static bool battery_gauge_enabled = false;
static BatteryChargeState battery_state;

static Animation *charging_animation;
static AnimationImplementation animation_implementation;
static int animating_tick_sizes[num_ticks];


static int min(int a, int b) {
    return a < b ? a : b;
}

static void calculate_tick_positions(Layer *layer) {
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

static void charging_animation_update(Animation *animation, const AnimationProgress progress) {
	float staggerness = 4.6;
	float staggered_percent = (staggerness+1)*progress/ANIMATION_NORMALIZED_MAX;
	for (int i=0; i<num_ticks; i++) {
		if (i < ticks_level) {
			float tick_percent = staggered_percent-staggerness*i/(num_ticks-1);
			animating_tick_sizes[i] = min(ticks_size, ticks_size*tick_percent);
		} else {
			animating_tick_sizes[i] = 0;
		}
	}
	layer_mark_dirty(ticks_layer);
}

static void ticks_update_proc(Layer *layer, GContext *ctx) {
	graphics_context_set_stroke_width(ctx, 3);
	graphics_context_set_stroke_color(ctx, ticks_colour);

	bool should_animate = battery_gauge_enabled && animation_is_scheduled(charging_animation);
	// APP_LOG(APP_LOG_LEVEL_DEBUG, "redrawing ticks level %i, percentage %i", ticks_level, battery_state.charge_percent);

	for (int i=0; i<num_ticks; i++) {
		GPoint pos = tick_positions[i];
		bool should_fill = (i < ticks_level);

		if (should_fill) {
			graphics_context_set_fill_color(ctx, ticks_colour);
			if (!should_animate) {
				graphics_fill_circle(ctx, pos, ticks_size);
			} else {
				graphics_fill_circle(ctx, pos, animating_tick_sizes[i]);
			}
		} else {
			if (ticks_size > 1) { // leave blank when tick size too small
				graphics_context_set_fill_color(ctx, get_bg_colour());
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

void init_ticks(Layer *layer) {
	ticks_layer = layer;

	calculate_tick_positions(ticks_layer);
	layer_set_update_proc(ticks_layer, ticks_update_proc);
}

void update_battery_ticks(BatteryChargeState charge_state) {
	battery_state = charge_state;

	if (battery_gauge_enabled) {
		ticks_level = battery_state.charge_percent*0.01*num_ticks;
	} else {
		ticks_level = num_ticks;
	}

	layer_mark_dirty(ticks_layer);
}

void animate_charging_indicator() {
	if (charging_animation) {
		// avoid double animation
		animation_unschedule(charging_animation);
		animation_destroy(charging_animation);
	}
	charging_animation = animation_create();
	animation_set_duration(charging_animation, 1000);
	animation_set_delay(charging_animation, 0);
	animation_set_curve(charging_animation, AnimationCurveEaseInOut);
	animation_implementation = (AnimationImplementation) {
		.update = charging_animation_update
	};
	animation_set_implementation(charging_animation, &animation_implementation);
	animation_schedule(charging_animation);
}

void update_tick_settings(GColor colour, int size, bool battery_enabled) {
	ticks_colour = colour;
	ticks_size = size;
	battery_gauge_enabled = battery_enabled;

	layer_mark_dirty(ticks_layer);
}
