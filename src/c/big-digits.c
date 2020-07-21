#include <pebble.h>
#include "big-digits.h"
#include "graphics.h"

#include "gbitmap_color_palette_manipulator.h"

#define TOTAL_TIME_DIGITS 4

const int DIGIT_IMAGE_RESOURCE_IDS[] = {
	RESOURCE_ID_CENTURY_GOTHIC_0, RESOURCE_ID_CENTURY_GOTHIC_1, 
	RESOURCE_ID_CENTURY_GOTHIC_2, RESOURCE_ID_CENTURY_GOTHIC_3, 
	RESOURCE_ID_CENTURY_GOTHIC_4, RESOURCE_ID_CENTURY_GOTHIC_5, 
	RESOURCE_ID_CENTURY_GOTHIC_6, RESOURCE_ID_CENTURY_GOTHIC_7, 
	RESOURCE_ID_CENTURY_GOTHIC_8, RESOURCE_ID_CENTURY_GOTHIC_9
};

static GColor time_colour;

static GBitmap *s_digits_images[TOTAL_TIME_DIGITS];
static BitmapLayer *s_digits_layers[TOTAL_TIME_DIGITS];

static Layer *sibling_layer;

static void unload_digit_image_from_slot(int slot_number) {
	BitmapLayer** layer = &s_digits_layers[slot_number];
	GBitmap** bitmap = &s_digits_images[slot_number];
	if (*layer) {
		layer_remove_from_parent(bitmap_layer_get_layer(*layer));
		bitmap_layer_destroy(*layer);
		*layer = NULL;
	}

	if (*bitmap) {
		gbitmap_destroy(*bitmap);
		*bitmap = NULL;
	}
}

// Loads the digit image from the application's resources and
// displays it on-screen in the correct location.
// Each slot is a quarter of the screen.
static void load_digit_image_into_slot(int slot_number, int digit_value) {
	unload_digit_image_from_slot(slot_number);

	GBitmap** bitmap = &s_digits_images[slot_number];
	*bitmap = gbitmap_create_with_resource(DIGIT_IMAGE_RESOURCE_IDS[digit_value]);

	#ifdef PBL_COLOR
	replace_gbitmap_color(GColorBlack, time_colour, *bitmap, NULL);
	#endif

	GRect bounds = get_window_bounds();
	GRect tile_bounds = gbitmap_get_bounds(*bitmap);

	const int tracking = -5;

	const int x_offset = bounds.size.w / 2 - (tile_bounds.size.w + tracking/2);
	const int y_offset = bounds.size.h / 2 - tile_bounds.size.h;

	GRect frame = GRect(
		x_offset + ((slot_number % 2) * (tile_bounds.size.w + tracking)),
		y_offset + ((slot_number / 2) * tile_bounds.size.h),
		tile_bounds.size.w, tile_bounds.size.h);

	BitmapLayer** layer = &s_digits_layers[slot_number];
	*layer = bitmap_layer_create(frame);

	bitmap_layer_set_compositing_mode(*layer, GCompOpSet);
	bitmap_layer_set_bitmap(*layer, *bitmap);

	// layer_add_child(window_layer, bitmap_layer_get_layer(*layer));
	layer_insert_below_sibling(bitmap_layer_get_layer(*layer), sibling_layer);
}

static void display_value(unsigned short value, unsigned short row_number, bool show_first_leading_zero) {
	value = value % 100; // Maximum of two digits per row.

	// Column order is: | Column 0 | Column 1 |
	// (We process the columns in reverse order because that makes
	// extracting the digits from the value easier.)
	for (int column_number = 1; column_number >= 0; column_number--) {
		int slot_number = (row_number * 2) + column_number;
		if (!((value == 0) && (column_number == 0) && !show_first_leading_zero)) {
			load_digit_image_into_slot(slot_number, value % 10);
		} else {
			unload_digit_image_from_slot(slot_number);
		}
		value = value / 10;
	}
}

void init_digits(Layer *layer) {
	sibling_layer = layer;
}

void set_digits_hour(unsigned short value) {
	display_value(value, 0, false);
}

void set_digits_minute(unsigned short value) {
	display_value(value, 1, true);
}

void set_digits_colour(GColor colour) {
	time_colour = colour;
}

void destroy_digits_layers() {
	for (int i=0; i<TOTAL_TIME_DIGITS; i++) {
		unload_digit_image_from_slot(i);
	}
}
