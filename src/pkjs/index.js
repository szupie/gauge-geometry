const Clay = require('pebble-clay');
const clayConfig = require('./config.json');
const customClay = require('./custom-clay');
const clay = new Clay(clayConfig, customClay);

var tempEnabled;
var tempUnits;
var tempFeelsLike;
var weatherProvider;
var weatherAPIKey;

var weatherReattemptTimer;
function queueReattempt(callback, reattemptDelay) {
	clearTimeout(weatherReattemptTimer);
	weatherReattemptTimer = setTimeout(callback, reattemptDelay);
}

function createRequest(url, callback, attempt) {
	const xhr = new XMLHttpRequest();
	const type = "GET";
	function onError(e) {
		console.error('Error: '+e);
		// Uncomment to see request url; exposes API key in logs
		// console.error('Request: '+url);
		console.error('Response: '+xhr.response);
		
		const maxAttempts = 3;
		if (!attempt) attempt = 1;
		if (attempt < maxAttempts) {
			attempt++;
			const reattemptDelay = Math.pow(2, attempt)*1000;
			queueReattempt(function() {
				console.log('Retrying (attempt '+attempt+' of '+maxAttempts+')');
				createRequest(url, callback, attempt);
			}, reattemptDelay);
		}
	}
	xhr.onloadend = function() {
		if (xhr.status == 200) {
			try {
				callback(xhr.response);
			} catch (e) {
				onError(e);
			}
		} else {
			onError(xhr.statusText);
		}
	};
	xhr.open(type, url);
	xhr.send();
};

function requestOwm(lat, lon, callback) {
	const units = (tempUnits === 'c') ? 'metric' : 'imperial';
	const url = 'http://api.openweathermap.org/data/2.5/onecall?lat='+lat+'&lon='+lon+'&units='+units+'&appid='+weatherAPIKey+'&exclude=hourly,minutely,alerts';

	createRequest(url, function (responseText) {
		const json = JSON.parse(responseText);
		var now, min, max;
		if (tempFeelsLike) {
			now = json.current.feels_like;
		} else {
			now = json.current.temp;
		}
		min = json.daily[0].temp.min;
		max = json.daily[0].temp.max;
		callback([now, min, max]);
	});
}

function requestTomorrow(lat, lon, callback) {
	const units = (tempUnits === 'c') ? 'metric' : 'imperial';
	const tempField = tempFeelsLike ? 'temperatureApparent' : 'temperature';
	const query = 'location='+lat+'%2C'+lon+'&units='+units+'&apikey='+weatherAPIKey;

	const nowEndpoint = 'https://api.tomorrow.io/v4/timelines?fields='+tempField+'&timesteps=current&'+query;
	const dailyEndpoint = 'https://api.tomorrow.io/v4/timelines?fields='+tempField+'Min&fields='+tempField+'Max&timesteps=1d&'+query;

	createRequest(nowEndpoint, function (responseText) {
		const json = JSON.parse(responseText);
		const now = json.data.timelines[0].intervals[0].values[tempField];

		createRequest(dailyEndpoint, function (responseText) {
			const json = JSON.parse(responseText);
			const minMax = json.data.timelines[0].intervals[0].values;
			const min = minMax[tempField+'Min'];
			const max = minMax[tempField+'Max'];
			callback([now, min, max]);
		});
	});
}

function requestWeatherbit(lat, lon, callback) {
	const units = (tempUnits === 'c') ? 'M' : 'I';
	const query = 'lat='+lat+'&lon='+lon+'&units='+units+'&key='+weatherAPIKey;

	const nowEndpoint = 'https://api.weatherbit.io/v2.0/current?'+query;
	const dailyEndpoint = 'https://api.weatherbit.io/v2.0/forecast/daily?days=1&'+query;

	createRequest(nowEndpoint, function (responseText) {
		const json = JSON.parse(responseText);
		var now;
		if (tempFeelsLike) {
			now = json.data[0].app_temp;
		} else {
			now = json.data[0].temp;
		}

		createRequest(dailyEndpoint, function (responseText) {
			const json = JSON.parse(responseText);
			var min, max;
			if (tempFeelsLike) {
				min = json.data[0].app_min_temp;
				max = json.data[0].app_max_temp;
			} else {
				min = json.data[0].min_temp;
				max = json.data[0].max_temp;
			}
			callback([now, min, max]);
		});
	});
}

function locationSuccess(pos) {
	console.log('Retrieving weather');
	if (weatherAPIKey) {
		const lat = pos.coords.latitude;
		const lon = pos.coords.longitude;
		var weatherFunction;
		switch (weatherProvider) {
			case 'owm':
				weatherFunction = requestOwm;
				break;
			case 'tomorrow':
				weatherFunction = requestTomorrow;
				break;
			case 'weatherbit':
				weatherFunction = requestWeatherbit;
				break;
		}

		function callback(nowMinMaxTuple) {
			const roundedTuple = nowMinMaxTuple.map(function (val) {
				return Math.round(val);
			});
			const now = roundedTuple[0];
			const min = roundedTuple[1];
			const max = roundedTuple[2];

			console.log('Current temp: '+now+'; min: '+min+'; max: '+max);

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
		}

		weatherFunction(lat, lon, callback);
	}
}

function locationError(err) {
	console.log('Error requesting location!');
}

function getWeather() {
	clearTimeout(weatherReattemptTimer);
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

	const tempProviderChanged = (weatherProvider !== claySettings['WEATHER_PROVIDER'].value);
	const tempUnitsChanged = (tempUnits !== claySettings['TEMP_UNIT'].value);
	const forceTempUpdate = claySettings['UPDATE_WEATHER_ON_CONFIG'].value;

	tempEnabled = claySettings['TEMP_ENABLED'].value;
	tempUnits = claySettings['TEMP_UNIT'].value;
	tempFeelsLike = claySettings['TEMP_FEELS_LIKE'].value;
	weatherProvider = claySettings['WEATHER_PROVIDER'].value;
	weatherAPIKey = claySettings['WEATHER_API_KEY'].value;

	if (tempProviderChanged || tempUnitsChanged || forceTempUpdate) {
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
			tempFeelsLike = localStorageSettings['TEMP_FEELS_LIKE'];
			weatherProvider = localStorageSettings['WEATHER_PROVIDER'];
			weatherAPIKey = localStorageSettings['WEATHER_API_KEY'];
		}
		// Uncomment next line and listen to logs on phone to get a copy of
		// the html code of the config page for debugging
		// console.log(clay.generateUrl());
	}
);

Pebble.addEventListener('appmessage', function(e) {
	getWeather();
});
