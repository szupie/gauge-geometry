#pragma once
#include <pebble.h>

typedef struct ClaySettings {
	GColor BgColour;
	GColor TimeColour;
	GColor DateColour;
	GColor HourHandColour;
	GColor MinuteHandColour;
	int HandsShape;
	GColor TicksColour;
	int TicksSize;
	bool BatteryGaugeEnabled;
	bool TempEnabled;
	char TempUnit;
	GColor TempNowColour;
	GColor TempRangeColour;
} ClaySettings;
ClaySettings settings;

void handle_settings_received(DictionaryIterator *iterator, void *context);
void load_settings();
