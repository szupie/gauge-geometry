#include "settings.h"
#include "graphics.h"
#include "weather.h"

uint32_t PERSIST_KEY_SETTINGS = 5;

enum SettingName {
	BG_COLOUR,
	TIME_COLOUR,
	DATE_COLOUR,
	HOUR_HAND_COLOUR,
	MINUTE_HAND_COLOUR,
	HANDS_SHAPE,
	TICKS_COLOUR,
	TICKS_SIZE,
	BATTERY_GAUGE_ENABLED,
	TEMP_ENABLED,
	TEMP_UNIT,
	TEMP_NOW_COLOUR,
	TEMP_RANGE_COLOUR,
};

enum SettingType {
	COLOUR_SETTING,
	BOOL_SETTING,
	INT_SETTING
};

// Save the settings to persistent storage
static void save_settings() {
	persist_write_data(PERSIST_KEY_SETTINGS, &settings, sizeof(settings));
}

static void load_default_settings() {
	#if defined(PBL_COLOR)
	settings.BgColour = GColorWhite;
	settings.TimeColour = GColorBlack;
	settings.DateColour = GColorOxfordBlue;

	settings.HourHandColour = GColorDarkCandyAppleRed;
	settings.MinuteHandColour = GColorCobaltBlue;

	settings.TicksColour = GColorOxfordBlue;

	settings.TempNowColour = GColorDarkCandyAppleRed;
	settings.TempRangeColour = GColorBrilliantRose;
	#else
	settings.BgColour = GColorWhite;
	settings.TimeColour = GColorBlack;
	settings.DateColour = GColorBlack;

	settings.HourHandColour = GColorBlack;
	settings.MinuteHandColour = GColorLightGray;

	settings.TicksColour = GColorBlack;

	settings.TempNowColour = GColorBlack;
	settings.TempRangeColour = GColorLightGray;
	#endif

	settings.HandsShape = '0';
	settings.TicksSize = 2;
	settings.BatteryGaugeEnabled = true;
	settings.TempEnabled = false;
}

static uint32_t get_setting_key(enum SettingName setting) {
	switch (setting) {
		case BG_COLOUR:
			return MESSAGE_KEY_BG_COLOUR;
		case TIME_COLOUR:
			return MESSAGE_KEY_TIME_COLOUR;
		case DATE_COLOUR:
			return MESSAGE_KEY_DATE_COLOUR;
		case HOUR_HAND_COLOUR:
			return MESSAGE_KEY_HOUR_HAND_COLOUR;
		case MINUTE_HAND_COLOUR:
			return MESSAGE_KEY_MINUTE_HAND_COLOUR;
		case HANDS_SHAPE:
			return MESSAGE_KEY_HANDS_SHAPE;
		case TICKS_COLOUR:
			return MESSAGE_KEY_TICKS_COLOUR;
		case TICKS_SIZE:
			return MESSAGE_KEY_TICKS_SIZE;
		case BATTERY_GAUGE_ENABLED:
			return MESSAGE_KEY_BATTERY_GAUGE_ENABLED;
		case TEMP_ENABLED:
			return MESSAGE_KEY_TEMP_ENABLED;
		case TEMP_UNIT:
			return MESSAGE_KEY_TEMP_UNIT;
		case TEMP_NOW_COLOUR:
			return MESSAGE_KEY_TEMP_NOW_COLOUR;
		case TEMP_RANGE_COLOUR:
			return MESSAGE_KEY_TEMP_RANGE_COLOUR;
		default:
			return 0;
	}
}

static enum SettingType get_setting_type(enum SettingName setting) {
	switch (setting) {
		case BG_COLOUR:
		case TIME_COLOUR:
		case DATE_COLOUR:
		case HOUR_HAND_COLOUR:
		case MINUTE_HAND_COLOUR:
		case TICKS_COLOUR:
		case TEMP_NOW_COLOUR:
		case TEMP_RANGE_COLOUR:
			return COLOUR_SETTING;
		case BATTERY_GAUGE_ENABLED:
		case TEMP_ENABLED:
			return BOOL_SETTING;
		case HANDS_SHAPE:
		case TICKS_SIZE:
		case TEMP_UNIT:
			return INT_SETTING;
		default:
			return 0;
	}
}

static void * get_setting_storage(enum SettingName setting) {
	switch (setting) {
		case BG_COLOUR:
			return &settings.BgColour;
		case TIME_COLOUR:
			return &settings.TimeColour;
		case DATE_COLOUR:
			return &settings.DateColour;
		case HOUR_HAND_COLOUR:
			return &settings.HourHandColour;
		case MINUTE_HAND_COLOUR:
			return &settings.MinuteHandColour;
		case HANDS_SHAPE:
			return &settings.HandsShape;
		case TICKS_COLOUR:
			return &settings.TicksColour;
		case TICKS_SIZE:
			return &settings.TicksSize;
		case BATTERY_GAUGE_ENABLED:
			return &settings.BatteryGaugeEnabled;
		case TEMP_ENABLED:
			return &settings.TempEnabled;
		case TEMP_UNIT:
			return &settings.TempUnit;
		case TEMP_NOW_COLOUR:
			return &settings.TempNowColour;
		case TEMP_RANGE_COLOUR:
			return &settings.TempRangeColour;
		default:
			return 0;
	}
}

static void extract_and_set(DictionaryIterator *iterator, enum SettingName setting) {
	Tuple *tuple = dict_find(iterator, get_setting_key(setting));
	if (tuple) {
		void *setting_storage = get_setting_storage(setting);
		switch (get_setting_type(setting)) {
			case COLOUR_SETTING:
				*(GColor*)setting_storage = GColorFromHEX(tuple->value->int32);
				break;
			case BOOL_SETTING:
				*(bool*)setting_storage = tuple->value->int32 == 1;
				break;
			case INT_SETTING:
				*(int*)setting_storage = tuple->value->int32;
				break;
		}
		
	}
}

void handle_settings_received(DictionaryIterator *iterator, void *context) {
	extract_and_set(iterator, BG_COLOUR);
	extract_and_set(iterator, TIME_COLOUR);
	extract_and_set(iterator, DATE_COLOUR);

	extract_and_set(iterator, HOUR_HAND_COLOUR);
	extract_and_set(iterator, MINUTE_HAND_COLOUR);
	extract_and_set(iterator, HANDS_SHAPE);

	extract_and_set(iterator, TICKS_COLOUR);
	extract_and_set(iterator, TICKS_SIZE);
	extract_and_set(iterator, BATTERY_GAUGE_ENABLED);

	extract_and_set(iterator, TEMP_ENABLED);
	extract_and_set(iterator, TEMP_UNIT);
	extract_and_set(iterator, TEMP_NOW_COLOUR);
	extract_and_set(iterator, TEMP_RANGE_COLOUR);

	save_settings();

	enable_temp(settings.TempEnabled);
	check_temp_unit_change(settings.TempUnit);

	update_style();
}


void load_settings() {
	load_default_settings();
	persist_read_data(PERSIST_KEY_SETTINGS, &settings, sizeof(settings));
}
