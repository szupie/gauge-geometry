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

static GBitmap *bitmaps[TOTAL_TIME_DIGITS];
static BitmapLayer *layers[TOTAL_TIME_DIGITS];
static unsigned short digit_values[TOTAL_TIME_DIGITS] = {
	UNDEFINED_DIGIT, UNDEFINED_DIGIT, UNDEFINED_DIGIT, UNDEFINED_DIGIT
};

static PropertyAnimation *prop_anims[TOTAL_TIME_DIGITS];
static Animation *anims[TOTAL_TIME_DIGITS];
static bool init_animated = false;


// Loads the digit image from the application's resources and
// displays it on-screen in the correct location.
// Each slot is a quarter of the screen.
static void load_digit_image_into_slot(int slot, int digit_value) {
	gbitmap_destroy(bitmaps[slot]);

	if (digit_value != UNDEFINED_DIGIT) {
		bitmaps[slot] = gbitmap_create_with_resource(
			DIGIT_IMAGE_RESOURCE_IDS[digit_value]
		);
	} else {
		bitmaps[slot] = gbitmap_create_blank(GSize(0, 0), GBitmapFormat1Bit);
	}

	bitmap_layer_set_bitmap(layers[slot], bitmaps[slot]);

	#ifdef PBL_COLOR
	replace_gbitmap_color(GColorBlack, time_colour, bitmaps[slot], NULL);
	#elif PBL_BW
	bitmap_layer_set_compositing_mode(layers[slot], GCompOpAnd);
	if (gcolor_equal(time_colour, GColorWhite)) {
		bitmap_layer_set_compositing_mode(layers[slot], GCompOpSet);
	}
	#endif

	if (!init_animated) {
		// initiate to bottom of frame
		Layer *digit_layer = bitmap_layer_get_layer(layers[slot]);
		GRect start_bounds = layer_get_bounds(digit_layer);
		start_bounds.origin.y = start_bounds.size.h;
		layer_set_bounds(digit_layer, start_bounds);

		GPoint finish = GPoint(0, 0);

		// set animation to frame top
		prop_anims[slot] = property_animation_create_bounds_origin(
			digit_layer, NULL, &finish
		);
		anims[slot] = property_animation_get_animation(prop_anims[slot]);
		animation_set_curve(anims[slot], AnimationCurveEaseOut);
		animation_set_delay(anims[slot], 100+40*slot);
		animation_set_duration(anims[slot], 250);

		animation_schedule(anims[slot]);
	}
}

static void update_slot(int slot_number) {
	int digit_value = digit_values[slot_number];
	if (digit_value != UNDEFINED_DIGIT) {
		// unload digit if it is leading 0 of hour
		if (digit_value == 0 && slot_number == 0) {
			load_digit_image_into_slot(slot_number, UNDEFINED_DIGIT);
		} else {
			load_digit_image_into_slot(slot_number, digit_value);
		}
	}
}

static void set_row(unsigned short row_number, unsigned short value) {
	// extract and display digits by reading units digit,
	// shifting decimal place and rereading
	for (int column_number = 1; column_number >= 0; column_number--) {
		int slot_number = (row_number * 2) + column_number;
		int current_digit = value % 10;

		digit_values[slot_number] = current_digit;
		update_slot(slot_number);

		// shift tens digit into units
		value = value / 10;
	}
}

void init_digits(Layer *parent_layer) {
	// load sample digit bitmap to determine size
	bitmaps[0] = gbitmap_create_with_resource(DIGIT_IMAGE_RESOURCE_IDS[0]);

	// determine positions of digits
	GRect window_bounds = layer_get_bounds(parent_layer);
	GRect tile_bounds = gbitmap_get_bounds(bitmaps[0]);

	// spacing for emery
	#if PBL_DISPLAY_HEIGHT > 180
	const int tracking = -2;
	const int leading = EMERY_LEADING;
	#else

	const int tracking = -5;
	const int leading = 0;
	#endif

	const int x_offset = window_bounds.size.w/2 - (tile_bounds.size.w + tracking/2);
	const int y_offset = (window_bounds.size.h-leading)/2 - tile_bounds.size.h;

	for (int slot_number=0; slot_number<TOTAL_TIME_DIGITS; slot_number++) {
		GRect frame = GRect(
			x_offset + ((slot_number % 2) * (tile_bounds.size.w + tracking)),
			y_offset + ((slot_number / 2) * (tile_bounds.size.h + leading)),
			tile_bounds.size.w, 
			tile_bounds.size.h
		);

		layers[slot_number] = bitmap_layer_create(frame);
		bitmap_layer_set_compositing_mode(layers[slot_number], GCompOpSet);

		// insert layers
		Layer *digit_layer = bitmap_layer_get_layer(layers[slot_number]);
		layer_add_child(parent_layer, digit_layer);
	}
}

void set_digits(unsigned short hour, unsigned short minute) {
	set_digits_hour(hour);
	set_digits_minute(minute);
	init_animated = true;
}

void set_digits_hour(unsigned short value) {
	set_row(0, value);
}

void set_digits_minute(unsigned short value) {
	set_row(1, value);
}

void set_digits_colour(GColor colour) {
	time_colour = colour;

	// reload bitmaps to reset replaced colours
	for (int slot_number=0; slot_number<TOTAL_TIME_DIGITS; slot_number++) {
		update_slot(slot_number);
	}
}

void destroy_digits() {
	for (int i=0; i<TOTAL_TIME_DIGITS; i++) {
		gbitmap_destroy(bitmaps[i]);
		layer_remove_from_parent(bitmap_layer_get_layer(layers[i]));
		bitmap_layer_destroy(layers[i]);
		layers[i] = NULL;
	}
}
