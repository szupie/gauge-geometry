#include <pebble.h>
#include "gbitmap_color_palette_manipulator.h"

#include "enamel.h"
#include <pebble-events/pebble-events.h>

static Window *s_main_window;
static Layer* window_layer;

static TextLayer *s_day_layer, *s_date_layer, *s_day_shadow_layer, *s_date_shadow_layer;
static Layer *s_ticks_canvas, *s_hands_layer, *s_temp_range_layer, *s_temp_now_layer;

static BatteryChargeState battery_state;

static EventHandle s_settings_updated_handle;

// static int temp_now, temp_min, temp_max;
uint32_t persist_temp_now = 1;
uint32_t persist_temp_min = 2;
uint32_t persist_temp_max = 3;

static GColor bg_colour;
static GColor time_colour;
static GColor date_colour;
static GColor hour_hand_colour;
static GColor minute_hand_colour;
static GColor ticks_colour;
static int ticks_size;

static bool battery_gauge_enabled;
static bool temp_enabled;
static GColor temp_range_colour;
static GColor temp_now_colour;

#define RING_INSET 8
#define TEMP_RANGE_WIDTH 12
#define TEMP_NOW_RADIUS 6
#define TEMP_NOW_STROKE 6
#define TEXT_SHADOW_OFFSET 1
#define HOUR_HAND_THICKNESS 8
#define MINUTE_HAND_THICKNESS 6


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

static GPath *s_minute_hand, *s_hour_hand;

#define TOTAL_TIME_DIGITS 4

const int DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_CENTURY_GOTHIC_0, RESOURCE_ID_CENTURY_GOTHIC_1, RESOURCE_ID_CENTURY_GOTHIC_2, RESOURCE_ID_CENTURY_GOTHIC_3, RESOURCE_ID_CENTURY_GOTHIC_4, RESOURCE_ID_CENTURY_GOTHIC_5, RESOURCE_ID_CENTURY_GOTHIC_6, RESOURCE_ID_CENTURY_GOTHIC_7, RESOURCE_ID_CENTURY_GOTHIC_8, RESOURCE_ID_CENTURY_GOTHIC_9
};

static GBitmap *s_digits_images[TOTAL_TIME_DIGITS];
static BitmapLayer *s_digits_layers[TOTAL_TIME_DIGITS];

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

  GRect bounds = layer_get_bounds(window_get_root_layer(s_main_window));
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
  layer_insert_below_sibling(bitmap_layer_get_layer(*layer), s_ticks_canvas);
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

static unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }

  // Converts "0" to "12"
  unsigned short display_hour = hour % 12;
  return display_hour ? display_hour : 12;
}

static void display_time(struct tm *tick_time) {
  int hours = get_display_hour(tick_time->tm_hour);
  int minutes = tick_time->tm_min;
  hours = 23;
  minutes = 54;

  display_value(hours, 0, false);
  display_value(minutes, 1, true);

  gpath_rotate_to(s_hour_hand, (TRIG_MAX_ANGLE * (((tick_time->tm_hour % 12) * 6) + (tick_time->tm_min / 10))) / (12 * 6));
  gpath_rotate_to(s_minute_hand, TRIG_MAX_ANGLE * tick_time->tm_min / 60);
  layer_mark_dirty(s_hands_layer);
}

static void display_date(struct tm *tick_time) {
  static char s_day_buffer[8], s_date_buffer[8];
  strftime(s_day_buffer, sizeof("ddd"), "%a", tick_time);
  text_layer_set_text(s_day_layer, s_day_buffer);
  text_layer_set_text(s_day_shadow_layer, s_day_buffer);

  strftime(s_date_buffer, sizeof("dd mmm"), "%e %b", tick_time);
  char *date_trimmed = s_date_buffer;
  if (date_trimmed[0] == ' ') date_trimmed++;
  text_layer_set_text(s_date_layer, "30 Jan");
  text_layer_set_text(s_date_shadow_layer, date_trimmed);
  // TODO try half width space

}

static void hands_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, minute_hand_colour);
  graphics_context_set_stroke_color(ctx, bg_colour);
  gpath_draw_outline(ctx, s_minute_hand);
  gpath_draw_filled(ctx, s_minute_hand);

  graphics_context_set_fill_color(ctx, hour_hand_colour);
  graphics_context_set_stroke_color(ctx, bg_colour);
  gpath_draw_outline(ctx, s_hour_hand);
  gpath_draw_filled(ctx, s_hour_hand);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  if (units_changed & DAY_UNIT) {
    display_date(tick_time);
  }

  display_time(tick_time);

  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void handle_battery(BatteryChargeState charge_state) {
  battery_state = charge_state;
  layer_mark_dirty(s_ticks_canvas);
}

static int minute_to_rad(int minute) {
  return DEG_TO_TRIGANGLE(minute*360/60);
}

static void temp_now_update_proc(Layer *layer, GContext *ctx) {
  if (persist_exists(persist_temp_now)) {
    int temp_now = persist_read_int(persist_temp_now);

    GRect bounds = layer_get_bounds(layer);
    int temp_angle = minute_to_rad(temp_now);

    // graphics_context_set_fill_color(ctx, GColorDarkCandyAppleRed);
    // graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, RING_INSET*2, temp_angle, temp_angle);

    GPoint pos = gpoint_from_polar(grect_inset(bounds, GEdgeInsets(RING_INSET)), GOvalScaleModeFitCircle, temp_angle);
    graphics_context_set_fill_color(ctx, bg_colour);
    graphics_context_set_stroke_color(ctx, temp_now_colour);
    graphics_context_set_stroke_width(ctx, TEMP_NOW_STROKE);
    graphics_fill_circle(ctx, pos, TEMP_NOW_RADIUS);
    graphics_draw_circle(ctx, pos, TEMP_NOW_RADIUS);
  }
}

static void temp_range_update_proc(Layer *layer, GContext *ctx) {
  if (persist_exists(persist_temp_min) && persist_exists(persist_temp_max)) {
    int temp_min = persist_read_int(persist_temp_min);
    int temp_max = persist_read_int(persist_temp_max);

    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_fill_color(ctx, temp_range_colour);
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, TEMP_RANGE_WIDTH, minute_to_rad(temp_min), minute_to_rad(temp_max));
  }
}

static void ticks_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_stroke_width(ctx, 3);
  graphics_context_set_stroke_color(ctx, ticks_colour);

  for (int i=0; i<12; i++) {
    int hour_angle = DEG_TO_TRIGANGLE(i*360/12);
    GPoint pos = gpoint_from_polar(grect_inset(bounds, GEdgeInsets(RING_INSET)), GOvalScaleModeFitCircle, hour_angle);

    if (!battery_gauge_enabled || i < battery_state.charge_percent*0.01*12) {
      graphics_context_set_fill_color(ctx, ticks_colour);
      graphics_fill_circle(ctx, pos, ticks_size);
    } else {
      if (ticks_size > 1) {
        graphics_context_set_fill_color(ctx, bg_colour);
        int ring_size = ticks_size;
        if (ticks_size > 2) {
          ring_size = ticks_size-1;
        }
        graphics_draw_circle(ctx, pos, ring_size);
        graphics_fill_circle(ctx, pos, ring_size);
        // check color TODO
      }
    }
  }
}

static void refresh_date_time() {
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);

  display_time(tick_time);
  display_date(tick_time);
}

static void update_style(void *context) {
  bg_colour = enamel_get_S_BG_COLOUR();
  time_colour = enamel_get_S_TIME_COLOUR();
  date_colour = enamel_get_S_DATE_COLOUR();
  hour_hand_colour = enamel_get_S_HOUR_HAND_COLOUR();
  minute_hand_colour = enamel_get_S_MINUTE_HAND_COLOUR();
  ticks_colour = enamel_get_S_TICKS_COLOUR();
  ticks_size = enamel_get_S_TICKS_SIZE();

  battery_gauge_enabled = enamel_get_S_BATTERY_GAUGE_ENABLED();
  temp_enabled = enamel_get_S_TEMP_ENABLED();
  temp_range_colour = enamel_get_S_TEMP_RANGE_COLOUR();
  temp_now_colour = enamel_get_S_TEMP_NOW_COLOUR();

  layer_set_hidden(s_temp_range_layer, !temp_enabled);
  layer_set_hidden(s_temp_now_layer, !temp_enabled);
  
  layer_mark_dirty(s_temp_range_layer);
  layer_mark_dirty(s_temp_now_layer);
  layer_mark_dirty(s_hands_layer);
  layer_mark_dirty(s_ticks_canvas);

  window_set_background_color(s_main_window, bg_colour);
  refresh_date_time();

  text_layer_set_text_color(s_day_layer, date_colour);
  text_layer_set_text_color(s_date_layer, date_colour);
  text_layer_set_text_color(s_day_shadow_layer, bg_colour);
  text_layer_set_text_color(s_date_shadow_layer, bg_colour);
}

static void set_text_style(TextLayer *layer) {
  GFont custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_POPPINS_16));
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_font(layer, custom_font);
  text_layer_set_text_alignment(layer, GTextAlignmentCenter);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read tuples for data
  Tuple *temp_now_tuple = dict_find(iterator, MESSAGE_KEY_TEMP_NOW);
  Tuple *temp_min_tuple = dict_find(iterator, MESSAGE_KEY_TEMP_MIN);
  Tuple *temp_max_tuple = dict_find(iterator, MESSAGE_KEY_TEMP_MAX);

  // If temp range is available
  if(temp_min_tuple && temp_max_tuple) {
    int temp_min = (int)temp_min_tuple->value->int32;
    int temp_max = (int)temp_max_tuple->value->int32;

    persist_write_int(persist_temp_min, temp_min);
    persist_write_int(persist_temp_max, temp_max);

    layer_mark_dirty(s_temp_range_layer);
  }

  if(temp_now_tuple) {
    int temp_now = (int)temp_now_tuple->value->int32;
    persist_write_int(persist_temp_now, temp_now);
    layer_mark_dirty(s_temp_now_layer);
  }
}

static void main_window_load(Window *window) {
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_hour_hand = gpath_create(&HOUR_HAND_POINTS);
  gpath_move_to(s_hour_hand, grect_center_point(&bounds));
  s_minute_hand = gpath_create(&MINUTE_HAND_POINTS);
  gpath_move_to(s_minute_hand, grect_center_point(&bounds));

  int font_size = 16;
  int center = 42;
  int bottom = bounds.size.h / 2 - font_size * 0;
  int width = 60;
  int left = center - (width / 2);

  s_day_layer = text_layer_create(GRect(left, bottom - (font_size*2), width, font_size*1.5));
  s_day_shadow_layer = text_layer_create(GRect(left+TEXT_SHADOW_OFFSET, bottom - (font_size*2)+TEXT_SHADOW_OFFSET, width, font_size*1.5));

  s_date_layer = text_layer_create(GRect(left, bottom - font_size, width, font_size*1.5));
  s_date_shadow_layer = text_layer_create(GRect(left+TEXT_SHADOW_OFFSET, bottom - font_size + TEXT_SHADOW_OFFSET, width, font_size*1.5));

  set_text_style(s_day_layer);
  set_text_style(s_day_shadow_layer);

  set_text_style(s_date_layer);
  set_text_style(s_date_shadow_layer);

  // temp range
  s_temp_range_layer = layer_create(bounds);
  layer_set_update_proc(s_temp_range_layer, temp_range_update_proc);

  // create ticks
  s_ticks_canvas = layer_create(bounds);
  layer_set_update_proc(s_ticks_canvas, ticks_update_proc);

  battery_state_service_subscribe(handle_battery);
  handle_battery(battery_state_service_peek());

  // temp current
  s_temp_now_layer = layer_create(bounds);
  layer_set_update_proc(s_temp_now_layer, temp_now_update_proc);

  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);

  layer_add_child(window_layer, s_temp_range_layer);
  layer_add_child(window_layer, s_ticks_canvas);
  layer_add_child(window_layer, s_temp_now_layer);
  layer_add_child(window_layer, s_hands_layer);

  layer_add_child(window_layer, text_layer_get_layer(s_day_shadow_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_shadow_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_day_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

  refresh_date_time();

  update_style(NULL);
  s_settings_updated_handle = enamel_settings_received_subscribe(update_style, NULL);

}

static void main_window_unload(Window *window) {
  enamel_settings_received_unsubscribe(s_settings_updated_handle);
  
  for (int i=0; i<TOTAL_TIME_DIGITS; i++) {
    unload_digit_image_from_slot(i);
  }

  // Destroy TextLayer
  text_layer_destroy(s_day_layer);
  text_layer_destroy(s_day_shadow_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_date_shadow_layer);

  layer_destroy(s_ticks_canvas);
  layer_destroy(s_hands_layer);
  gpath_destroy(s_hour_hand);
  gpath_destroy(s_minute_hand);
}

static void init() {
  // Initialize Enamel to register App Message handlers and restores settings
  enamel_init();

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  events_app_message_register_inbox_received(inbox_received_callback, NULL);
  const int inbox_size = 128;
  const int outbox_size = 128;
  events_app_message_request_inbox_size(inbox_size);
  events_app_message_request_outbox_size(outbox_size);
  events_app_message_open();

}

static void deinit() {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  window_destroy(s_main_window);

  // Deinit Enamel to unregister App Message handlers and save settings
  enamel_deinit();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
