#include <pebble.h>
#include "big-digits.h"
#include "graphics.h"

#include "gbitmap_color_palette_manipulator.h"

#define TOTAL_TIME_DIGITS 4
#define UNDEFINED_DIGIT 99

static const int DIGIT_IMAGE_RESOURCE_IDS[] = {
	RESOURCE_ID_CENTURY_GOTHIC_0, RESOURCE_ID_CENTURY_GOTHIC_1, 
	RESOURCE_ID_CENTURY_GOTHIC_2, RESOURCE_ID_CENTURY_GOTHIC_3, 
	RESOURCE_ID_CENTURY_GOTHIC_4, RESOURCE_ID_CENTURY_GOTHIC_5, 
	RESOURCE_ID_CENTURY_GOTHIC_6, RESOURCE_ID_CENTURY_GOTHIC_7, 
	RESOURCE_ID_CENTURY_GOTHIC_8, RESOURCE_ID_CENTURY_GOTHIC_9
};

static GColor time_colour;

static GBitmap *digit_bitmaps[10];
static BitmapLayer *digit_layers[TOTAL_TIME_DIGITS];
static unsigned short digit_values[TOTAL_TIME_DIGITS] = {UNDEFINED_DIGIT, UNDEFINED_DIGIT, UNDEFINED_DIGIT, UNDEFINED_DIGIT};

static PropertyAnimation *prop_anims[TOTAL_TIME_DIGITS];
static Animation *anims[TOTAL_TIME_DIGITS];
static bool init_animated = false;


static void unload_digit_image_from_slot(int slot_number) {
	bitmap_layer_set_bitmap(
		digit_layers[slot_number], 
		gbitmap_create_blank(GSize(0, 0), GBitmapFormat1Bit)
	);
}

// Loads the digit image from the application's resources and
// displays it on-screen in the correct location.
// Each slot is a quarter of the screen.
static void load_digit_image_into_slot(int slot_number, int digit_value) {
	GBitmap *bitmap_to_load = digit_bitmaps[digit_value];

	#ifdef PBL_COLOR
	replace_gbitmap_color(GColorBlack, time_colour, bitmap_to_load, NULL);
	#endif

	bitmap_layer_set_bitmap(digit_layers[slot_number], bitmap_to_load);

	if (!init_animated) {
		// initiate to bottom of frame
		Layer *digit_layer = bitmap_layer_get_layer(digit_layers[slot_number]);
		GRect start_bounds = layer_get_bounds(digit_layer);
		start_bounds.origin.y = start_bounds.size.h;
		layer_set_bounds(digit_layer, start_bounds);

		GPoint finish = GPoint(0, 0);

		// set animation to frame top
		prop_anims[slot_number] = property_animation_create_bounds_origin(digit_layer, NULL, &finish);
		anims[slot_number] = property_animation_get_animation(prop_anims[slot_number]);
		animation_set_curve(anims[slot_number], AnimationCurveEaseOut);
		animation_set_delay(anims[slot_number], 100+40*slot_number);
		animation_set_duration(anims[slot_number], 250);

		animation_schedule(anims[slot_number]);
	}
}

static void display_value(unsigned short value, unsigned short row_number) {
	// extract and display digits by reading units digit,
	// shifting decimal place and rereading
	for (int column_number = 1; column_number >= 0; column_number--) {
		int slot_number = (row_number * 2) + column_number;
		int current_digit = value % 10;

		// unload digit if it is leading 0 of hour
		if (current_digit == 0 && column_number == 0 && row_number == 0) {unload_digit_image_from_slot(slot_number);
		} else {
			// load digit into slot if different from existing digit
			if (digit_values[slot_number] == UNDEFINED_DIGIT || digit_values[slot_number] != current_digit) {
				load_digit_image_into_slot(slot_number, current_digit);
			}
		}

		digit_values[slot_number] = current_digit;

		// shift tens digit into units
		value = value / 10;
	}

}

void init_digits(Layer *parent_layer) {
	// load in bitmaps to memory
	for (int i=0; i<10; i++) {
		digit_bitmaps[i] = gbitmap_create_with_resource(DIGIT_IMAGE_RESOURCE_IDS[i]);
	}

	// determine positions of digits
	GRect window_bounds = layer_get_bounds(parent_layer);
	GRect tile_bounds = gbitmap_get_bounds(digit_bitmaps[0]);

	const int tracking = -5;
	const int x_offset = window_bounds.size.w/2 - (tile_bounds.size.w + tracking/2);
	const int y_offset = window_bounds.size.h/2 - tile_bounds.size.h;

	for (int slot_number = 0; slot_number < TOTAL_TIME_DIGITS; slot_number++) {
		GRect frame = GRect(
			x_offset + ((slot_number % 2) * (tile_bounds.size.w + tracking)),
			y_offset + ((slot_number / 2) * tile_bounds.size.h),
			tile_bounds.size.w, 
			tile_bounds.size.h
		);

		digit_layers[slot_number] = bitmap_layer_create(frame);
		bitmap_layer_set_compositing_mode(digit_layers[slot_number], GCompOpSet);

		// insert layers
		Layer *digit_layer = bitmap_layer_get_layer(digit_layers[slot_number]);
		layer_add_child(parent_layer, digit_layer);
	}

}

void set_digits(unsigned short hour, unsigned short minute) {
	set_digits_hour(hour);
	set_digits_minute(minute);
	init_animated = true;
}

void set_digits_hour(unsigned short value) {
	display_value(value, 0);
}

void set_digits_minute(unsigned short value) {
	display_value(value, 1);
}

void set_digits_colour(GColor colour) {
	time_colour = colour;
}

void destroy_digits() {
	for (int i=0; i<TOTAL_TIME_DIGITS; i++) {
		unload_digit_image_from_slot(i);
		layer_remove_from_parent(bitmap_layer_get_layer(digit_layers[i]));
		bitmap_layer_destroy(digit_layers[i]);
		digit_layers[i] = NULL;
	}
	for (int i=0; i<10; i++) {
		gbitmap_destroy(digit_bitmaps[i]);
	}
}
