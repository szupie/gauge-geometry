#include <pebble.h>

enum HandShape { DAUPHINE, BAGUETTE, PENCIL };

#define DAUPINE_HOUR_THICKNESS 8
#define DAUPINE_MINUTE_THICKNESS 6
static const GPathInfo DAUPHINE_HOUR_POINTS = {
	4, (GPoint []){
		{0, DAUPINE_HOUR_THICKNESS},
		{-DAUPINE_HOUR_THICKNESS, 0},
		{0, -46},
		{DAUPINE_HOUR_THICKNESS, 0}
	}
};
static const GPathInfo DAUPHINE_MINUTE_POINTS = {
	4, (GPoint []){
		{0, DAUPINE_MINUTE_THICKNESS},
		{-DAUPINE_MINUTE_THICKNESS, 0},
		{0, -80},
		{DAUPINE_MINUTE_THICKNESS, 0}
	}
};

static const GPathInfo BAGUETTE_HOUR_POINTS = {
	2, (GPoint []){
		{0, 0},
		{0, -46}
	}
};
static const GPathInfo BAGUETTE_MINUTE_POINTS = {
	2, (GPoint []){
		{0, 0},
		{0, -70}
	}
};

static const GPathInfo PENCIL_HOUR_POINTS = {
	2, (GPoint []){
		{0, 0},
		{0, -40}
	}
};
static const GPathInfo PENCIL_MINUTE_POINTS = {
	2, (GPoint []){
		{0, 0},
		{0, -65}
	}
};
