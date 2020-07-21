#include <pebble.h>
#include "graphics.h"
#include "weather.h"

#include "enamel.h"
#include <pebble-events/pebble-events.h>

static Window *s_main_window;

static EventHandle s_settings_updated_handle;


static unsigned short get_display_hour(unsigned short hour) {
	if (clock_is_24h_style()) {
		return hour;
	}

	// Converts "0" to "12"
	unsigned short display_hour = hour % 12;
	return display_hour ? display_hour : 12;
}

static void display_time(struct tm *tick_time) {
	int hour = get_display_hour(tick_time->tm_hour);
	int minute = tick_time->tm_min;

	update_time(hour, minute);
}

static void display_date(struct tm *tick_time) {
	static char s_day_buffer[8], s_date_buffer[8];
	strftime(s_day_buffer, sizeof("ddd"), "%a", tick_time);
	update_day_of_week(s_day_buffer);

	strftime(s_date_buffer, sizeof("dd mmm"), "%e %b", tick_time);
	char *date_trimmed = s_date_buffer;
	if (date_trimmed[0] == ' ') date_trimmed++;
	update_date_month(date_trimmed);
	// TODO try half width space

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

static void refresh_date_time() {
	time_t now = time(NULL);
	struct tm *tick_time = localtime(&now);

	display_time(tick_time);
	display_date(tick_time);
}

static void handle_settings_received(void *context) {
	update_style();
	refresh_date_time();
}

static void main_window_load(Window *window) {
	load_window(window);

	battery_state_service_subscribe(handle_battery_update);
	handle_battery_update(battery_state_service_peek());

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

	refresh_date_time();

	s_settings_updated_handle = enamel_settings_received_subscribe(handle_settings_received, NULL);
}

static void main_window_unload(Window *window) {
	enamel_settings_received_unsubscribe(s_settings_updated_handle);
	destroy_layers();
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

	events_app_message_register_inbox_received(handle_weather_update, NULL);
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
