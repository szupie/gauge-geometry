#include <pebble.h>
#include "graphics.h"
#include "weather.h"

#include "enamel.h"
#include <pebble-events/pebble-events.h>

static Window *main_window;

static EventHandle settings_updated_handle;

static bool last_charging_state;

// #define DEBUGGING_TIME
// #define DEMO_MODE


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
	static char day_buffer[5], date_buffer[10];
	strftime(day_buffer, 5, "%a", tick_time);
	update_day_of_week(day_buffer);

	#if defined(PBL_ROUND)
	char date_format[] = "%e %b";
	#elif defined(PBL_RECT)
	char date_format[] = "%e\n%b";
	#endif

	strftime(date_buffer, 10, date_format, tick_time);
	char *date_trimmed = date_buffer;

	// trim initial space
	if (date_trimmed[0] == ' ') date_trimmed++;

	// trim final "."
	int date_length = strlen(date_trimmed);
	if (date_trimmed[date_length-1] == '.') {
		date_trimmed[date_length-1] = '\0';
	}

	update_date_month(date_trimmed);
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

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
	if (units_changed & MINUTE_UNIT) {
		handle_minute_tick(tick_time, units_changed);
	}

	if(tick_time->tm_sec % 2 == 0) {
		animate_charging_indicator();
	}
}

static void handle_battery_change(BatteryChargeState charge_state) {
	update_battery_ticks(charge_state);

	bool charging_changed = (last_charging_state != charge_state.is_charging);
	last_charging_state = charge_state.is_charging;

	if (charging_changed) {
		if (charge_state.is_charging) {
			tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
		} else {
			tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
		}
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
}

#ifdef DEBUGGING_TIME
static time_t debug_now;
static struct tm *debug_time;
static void debug_cycle_dates() {
	debug_time->tm_wday = debug_time->tm_sec%7;
	debug_time->tm_mday = debug_time->tm_mday%31+1;
	debug_time->tm_mon = debug_time->tm_sec%12;

	display_date(debug_time);
}

static void handle_test(struct tm *tick_time, TimeUnits units_changed) {
	update_time((tick_time->tm_min+tick_time->tm_sec)%24, tick_time->tm_sec%60);

	enable_temp(true);
	int temp_min = rand() % 30 - 20;
	int temp_max = rand() % 50 + temp_min;
	update_temp_range(temp_min, temp_max);
	update_temp_now(tick_time->tm_sec-20);

	debug_now = time(NULL);
	debug_time = localtime(&debug_now);
	debug_cycle_dates();
}
#endif


static void main_window_load(Window *window) {
	setlocale(LC_ALL, ""); // use locale set by user in phone app
	load_window(window);

	#ifndef DEMO_MODE
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

	BatteryChargeState charge_state = battery_state_service_peek();
	// init charging state as opposite to force refresh
	last_charging_state = !charge_state.is_charging;
	handle_battery_change(charge_state);
	battery_state_service_subscribe(handle_battery_change);

	refresh_date_time();
	#else
	// static display in demo mode
	update_time(10, 8);
	update_day_of_week("Fri");
		#if defined(PBL_ROUND)
		update_date_month("11 Nov");
		#elif defined(PBL_RECT)
		update_date_month("11\nNov");
		#endif

	update_battery_ticks((BatteryChargeState){70, false, false});

	enable_temp(true);
	update_temp_range(15, 25);
	update_temp_now(18);
	#endif

	settings_updated_handle = enamel_settings_received_subscribe(handle_settings_received, NULL);

	// DEBUGGING
	#ifdef DEBUGGING_TIME
	debug_now = time(NULL);
	debug_time = localtime(&debug_now);
	tick_timer_service_subscribe(SECOND_UNIT, handle_test);
	#endif
}

static void main_window_unload(Window *window) {
	enamel_settings_received_unsubscribe(settings_updated_handle);
	destroy_layers();
}

static void init() {
	// Initialize Enamel to register App Message handlers and restores settings
	enamel_init();

	main_window = window_create();
	window_set_window_handlers(main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload,
	});
	window_stack_push(main_window, true);

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
	window_destroy(main_window);

	// Deinit Enamel to unregister App Message handlers and save settings
	enamel_deinit();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
