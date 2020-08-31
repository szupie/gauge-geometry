module.exports = function(minified) {
	var clayConfig = this;
	var _ = minified._;
	var $ = minified.$;
	var HTML = minified.HTML;

	clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
		clayConfig.getItemById('save-button').$element.set({
			'$position': 'sticky',
			'$bottom': '0',
			'$backgroundColor': '#333333'
		});

		// Add notch marks to slider input to make sizing options clearer
		const ticksSizeInput = $('input.slider', clayConfig.getItemByMessageKey('TICKS_SIZE').$element);
		const sliderCount = ticksSizeInput.get('@max');
		ticksSizeInput.set({
			'$background': `repeating-linear-gradient(to right, #666, #666 1px, transparent 1px, transparent calc(100% / ${sliderCount} - 1px), #666 calc(100% / ${sliderCount} - 1px), #666 calc(100% / ${sliderCount}))`,
			'$backgroundSize': 'calc(100% - 1.4rem) 1rem',
			'$backgroundPosition': 'center',
			'$backgroundRepeat': 'no-repeat'
		});

		const weatherEnabledToggle = clayConfig.getItemByMessageKey('TEMP_ENABLED');
		const weatherProviderInput = clayConfig.getItemByMessageKey('WEATHER_PROVIDER');
		const weatherElements = clayConfig.getItemsByGroup('weatherDetails');

		function updateWeatherElements() {
			if (weatherProviderInput.get() === "none") {
				weatherEnabledToggle.set(false);
				weatherElements.forEach(element => {
					element.disable();
				});
			} else {
				weatherEnabledToggle.set(true);
				weatherElements.forEach(element => {
					element.enable();
				});
			}
		}
		
		weatherProviderInput.on('change', function() {
			updateWeatherElements();
		});

		weatherEnabledToggle.hide();
		updateWeatherElements();
	});
	
};
