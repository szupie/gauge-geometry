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

		// Add notch marks to slider input to make sizing increments clearer
		const ticksSizeInput = $('input.slider', clayConfig.getItemByMessageKey('TICKS_SIZE').$element);
		const sliderCount = ticksSizeInput.get('@max');
		ticksSizeInput.set({
			'$background': `repeating-linear-gradient(to right, #666, #666 1px, transparent 1px, transparent calc(100% / ${sliderCount} - 1px), #666 calc(100% / ${sliderCount} - 1px), #666 calc(100% / ${sliderCount}))`,
			'$backgroundSize': 'calc(100% - 1.4rem) 1rem',
			'$backgroundPosition': 'center',
			'$backgroundRepeat': 'no-repeat'
		});

		// Style subsection headings
		$('.component-heading:not(:first-child)').each(item => {
			$(item).set('$paddingBottom', '0.5rem');
			$('h6', item).set({
				'$color': '#a4a4a4',
				'$textTransform': 'uppercase'
			});
		});
		$(':not(.component-heading) + .component-heading').each(item => {
			$('h6', item).set('$marginTop', '2rem');
		});

		// Weather request can only be triggered on save, 
		// so make button toggle hidden setting that gets reset every time
		const forceWeatherToggle = clayConfig.getItemByMessageKey('UPDATE_WEATHER_ON_CONFIG');
		const fetchWeatherButton = clayConfig.getItemById('fetchWeather');

		forceWeatherToggle.hide();
		forceWeatherToggle.set(false);
		$('button', fetchWeatherButton.$element).set({
			'$backgroundColor': '#666',
			'$marginBottom': '0',
			'$textTransform': 'none',
			'$paddingLeft': '1rem',
			'$paddingRight': '1rem',
			'$fontWeight': 'normal'
		});
		fetchWeatherButton.on('click', function() {
			forceWeatherToggle.set(true);
		});

		// Style diagram for temperature dial
		const tempUnitInput = clayConfig.getItemByMessageKey('TEMP_UNIT');
		const dialDiameter = 7;
		const markerThickness = 5;
		$('#temp_dial').set({
			$height: `${dialDiameter}rem`,
			$width: `${dialDiameter}rem`,
			$backgroundColor: '#666',
			$borderRadius: '100%',
			$margin: 'auto auto 1rem'
		});

		function updateTempDiagram() {
			const tempUnit = tempUnitInput.get();
			let tempAttr, angleMultiplier;
			if (tempUnit === 'c') {
				tempAttr = 'data-c';
				angleMultiplier = 6;
			} else {
				tempAttr = 'data-f';
				angleMultiplier = 3;
			}
			// extra $ marks necessary because they get dropped (escaped?)
			$('#temp_explain .celsius').set('$$$$show', tempUnit === 'c');
			$('#temp_explain .fahrenheit').set('$$$$show', tempUnit === 'f');

			$('#temp_dial .label').each((item, index) => {
				const degree = $(item).get(`@${tempAttr}`);
				const angle = degree*angleMultiplier;
				item.innerHTML = `<span class="text">${degree}°</span>`;
				$(item).set({
					$position: 'absolute',
					$paddingTop: '0.1em',
					$height: `${dialDiameter}rem`,
					$width: '1px',
					$left: `calc(50% + 2px - ${markerThickness}px / 2)`,
					$transform: `rotate(${angle}deg)`,
					$borderTop: `${markerThickness}px solid currentcolor`,
					$fontSize: '0.75rem'
				});
				$('.text', item).set({
					$display: 'inline-block',
					$margin: 'auto',
					$transform: `translateX(-50%) rotate(${-angle}deg)`
				});
			});
		}
		
		tempUnitInput.on('change', function() {
			updateTempDiagram();
		});
		updateTempDiagram();

		// Dis/Enable weather options based on provider selection
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
