const Clay = require('pebble-clay');
const clayConfig = require('./config.json');
const customClay = require('./custom-clay');
const clay = new Clay(clayConfig, customClay);

let weatherProvider;
let weatherAPIKey;
let tempUnits;
let tempEnabled;

function xhrRequest(url, type, callback) {
	const xhr = new XMLHttpRequest();
	xhr.onload = function () {
		callback(this.responseText);
	};
	xhr.open(type, url);
	xhr.send();
};

function getWeatherUrl(lat, lon) {
	const useMetric = (tempUnits === 'c');
	switch (weatherProvider) {
		case 'darksky': {
			const units = useMetric ? 'si' : 'us';
			return `https://api.darksky.net/forecast/${weatherAPIKey}/${lat},${lon}?units=${units}&exclude=minutely,hourly,alerts,flags`;
		}
		case 'owm': {
			const units = useMetric ? 'metric' : 'imperial';
			return `https://api.openweathermap.org/data/2.5/onecall?lat=${lat}&lon=${lon}&units=${units}&appid=${weatherAPIKey}&exclude=hourly,minutely`;
		}
	}
}

function parseCurrentMinMaxFor(json) {
	let now, min, max;
	switch (weatherProvider) {
		case 'darksky':
			now = json.currently.apparentTemperature;
			min = json.daily.data[0].apparentTemperatureLow;
			max = json.daily.data[0].apparentTemperatureHigh;
			break;
		case 'owm':
			now = json.current.feels_like;
			min = json.daily[0].temp.min;
			max = json.daily[0].temp.max;
			break;
	}
	return [now, min, max];
}

function locationSuccess(pos) {
	console.log('Retrieving weather');
	if (weatherAPIKey) {
		const url = getWeatherUrl(pos.coords.latitude, pos.coords.longitude);

		xhrRequest(url, 'GET', function(responseText) {
			// responseText contains a JSON object with weather info
			const json = JSON.parse(responseText);

			let now, min, max;
			[now, min, max] = 
				parseCurrentMinMaxFor(json).map(val => Math.round(val));

			console.log(`Current temp: ${now}; min: ${min}; max: ${max}`);

			// Assemble dictionary using our keys
			const dictionary = {
				'TEMP_NOW': now,
				'TEMP_MIN': min,
				'TEMP_MAX': max,
			};

			// Send to Pebble
			Pebble.sendAppMessage(dictionary,
				function(e) {
					console.log('Weather info sent to Pebble successfully!');
				},
				function(e) {
					console.log('Error sending weather info to Pebble!');
				}
			);
		});
	}
}

function locationError(err) {
	console.log('Error requesting location!');
}

function getWeather() {
	if (tempEnabled) {
		navigator.geolocation.getCurrentPosition(
			locationSuccess,
			locationError,
			{timeout: 15000, maximumAge: 60000}
		);
	}
}

Pebble.addEventListener('webviewclosed', function(e) {
	const claySettings = clay.getSettings(e.response, false);

	const tempUnitsChanged = (tempUnits !== claySettings['TEMP_UNIT'].value);

	tempEnabled = claySettings['TEMP_ENABLED'].value;
	tempUnits = claySettings['TEMP_UNIT'].value;
	weatherProvider = claySettings['WEATHER_PROVIDER'].value;
	weatherAPIKey = claySettings['WEATHER_API_KEY'].value;

	if (claySettings['UPDATE_WEATHER_ON_CONFIG'].value || tempUnitsChanged) {
		getWeather();
	}
});

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
	function(e) {
		console.log('PebbleKit JS ready!');

		if (localStorage.getItem('clay-settings') !== null) {
			const localStorageSettings = JSON.parse(localStorage.getItem('clay-settings'));
			tempEnabled = localStorageSettings['TEMP_ENABLED'];
			tempUnits = localStorageSettings['TEMP_UNIT'];
			weatherProvider = localStorageSettings['WEATHER_PROVIDER'];
			weatherAPIKey = localStorageSettings['WEATHER_API_KEY'];
		}
	}
);

Pebble.addEventListener('appmessage', function(e) {
	getWeather();
});
