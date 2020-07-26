var Clay = require('pebble-clay');
var clayConfig = require('./config.json');
var customClay = require('./custom-clay');
var clay = new Clay(clayConfig, customClay);

var weatherAPIKey;
var tempEnabled;

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  console.log('Retrieving weather')
  if (weatherAPIKey) {
    // Construct URL
    var url = `https://api.darksky.net/forecast/${weatherAPIKey}/${pos.coords.latitude},${pos.coords.longitude}?units=si`;

    xhrRequest(url, 'GET', 
      function(responseText) {
        // responseText contains a JSON object with weather info
        var json = JSON.parse(responseText);

        var tempNow = json.currently.apparentTemperature;
        console.log('Temperature is ' + tempNow);

        var tempMin = json.daily.data[0].apparentTemperatureLow;
        console.log('Min is ' + tempMin);

        var tempMax = json.daily.data[0].apparentTemperatureHigh;
        console.log('Max is ' + tempMax);

        // Assemble dictionary using our keys
        var dictionary = {
          'TEMP_NOW': tempNow,
          'TEMP_MIN': tempMin,
          'TEMP_MAX': tempMax,
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
    );
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
  clay.getSettings(e.response);
  const claySettings = clay.getSettings(e.response, false);
  weatherAPIKey = claySettings['WEATHER_API_KEY'].value;
  tempEnabled = claySettings['TEMP_ENABLED'].value;
  getWeather();
});

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');

    if (localStorage.getItem('clay-settings') !== null) {
      const claySettings = JSON.parse(localStorage.getItem('clay-settings'));
      weatherAPIKey = claySettings['WEATHER_API_KEY'];
      tempEnabled = claySettings['TEMP_ENABLED'];
    }
  }
);

Pebble.addEventListener('appmessage', function(e) {
  getWeather();
});
