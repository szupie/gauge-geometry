module.exports = function(minified) {
	var clayConfig = this;
	var _ = minified._;
	var $ = minified.$;
	var HTML = minified.HTML;

	clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
		clayConfig.getItemById('save-button').$element.set({
			$position: 'sticky',
			$bottom: '0',
			$backgroundColor: '#333333',
			$zIndex: 3
		});

		// Add notch marks to slider input to make sizing increments clearer
		const ticksSizeInput = $('input.slider', clayConfig.getItemByMessageKey('TICKS_SIZE').$element);
		const sliderCount = ticksSizeInput.get('@max');
		ticksSizeInput.set({
			$background: `repeating-linear-gradient(to right, #666, #666 1px, transparent 1px, transparent calc(100% / ${sliderCount} - 1px), #666 calc(100% / ${sliderCount} - 1px), #666 calc(100% / ${sliderCount}))`,
			$backgroundSize: 'calc(100% - 1.4rem) 0.75rem',
			$backgroundPosition: 'center',
			$backgroundRepeat: 'no-repeat'
		});

		// Style subsection headings
		$('.component-heading:not(:first-child)').each(item => {
			$(item).set('$paddingBottom', '0.5rem');
			$('h6', item).set({
				$color: '#a4a4a4',
				$textTransform: 'uppercase'
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
			$backgroundColor: '#666',
			$marginBottom: '0',
			$textTransform: 'none',
			$paddingLeft: '1rem',
			$paddingRight: '1rem',
			$fontWeight: 'normal'
		});
		fetchWeatherButton.on('click', function() {
			forceWeatherToggle.set(true);
		});

		// Style explanation for temperature dial
		const tempUnitInput = clayConfig.getItemByMessageKey('TEMP_UNIT');
		const dialDiameter = 7;
		const markerThickness = 5;

		const tempCurrentColourInput = clayConfig.getItemByMessageKey('TEMP_NOW_COLOUR');
		const tempRangeColourInput = clayConfig.getItemByMessageKey('TEMP_RANGE_COLOUR');
		const currentIndicatorDiameter = 12;

		$('.description', tempUnitInput.$element).set({
			$display: 'flex',
			$flexWrap: 'wrap',
			$justifyContent: 'center',
			$alignItems: 'center'
		});
		$('#temp_dial').set({
			$height: `${dialDiameter}rem`,
			$width: `${dialDiameter}rem`,
			$backgroundColor: '#666',
			$borderRadius: '100%',
			$position: 'relative'
		});
		
		$('#temp_dial .indicator.current').set({
			$height: `${currentIndicatorDiameter}px`,
			$width: `${currentIndicatorDiameter}px`,
			$backgroundColor: '#666',
			$border: `4px solid transparent`,
			$borderRadius: '100%',
			$position: 'absolute',
			$left: `calc(50% - ${currentIndicatorDiameter/2}px)`,
			$marginTop: '-2px',
			$transformOrigin: `${currentIndicatorDiameter/2}px calc(${dialDiameter/2}rem + 2px)`,
			$zIndex: '1'
		});
		$('#temp_dial .indicator.range').set({
			$height: `${dialDiameter}rem`,
			$width: `${dialDiameter}rem`,
			$borderWidth: '6px',
			$borderStyle: 'solid',
			$borderRadius: '100%',
			$position: 'absolute',
			$clipPath: 'polygon(50% 0, 50% 50%, 136% 0)'
		});
	
		$('#temp_sample').set({
			$marginLeft: '0.75rem',
			$fontSize: '0.75rem',
			$lineHeight: '1.5em',
			$opacity: '0.6'
		});
		$('#temp_sample h6').set({
			$textTransform: 'uppercase',
			$lineHeight: 'inherit'
		});
		$('#temp_explain').set({
			$flex: '1 0 100%',
			$marginTop: '1rem'
		});

		function numToHexColour(num) {
			return '#'+num.toString(16).padStart(6, "0");
		}
		function updateTempCurrentColour() {
			const currentColour = numToHexColour(tempCurrentColourInput.get());
			$('#temp_dial .indicator.current').set('$borderColor', currentColour);
		}
		function updateTempRangColour() {
			const rangeColour = numToHexColour(tempRangeColourInput.get());
			$('#temp_dial .indicator.range').set('$borderColor', rangeColour);
		}
		updateTempCurrentColour();
		updateTempRangColour();
		tempCurrentColourInput.on('change', updateTempCurrentColour);
		tempRangeColourInput.on('change', updateTempRangColour);

		function drawTempDial() {
			const tempUnit = tempUnitInput.get();
			let scaleMultiplier, angleMultiplier, indicatorRotation;
			if (tempUnit === 'c') {
				scaleMultiplier = 5;
				angleMultiplier = 6;
				indicatorRotation = 90;
			} else {
				scaleMultiplier = 10;
				angleMultiplier = 3;
				indicatorRotation = 180;
			}
			// extra $ marks necessary because they get dropped (escaped?)
			$('.celsius', tempUnitInput.$element).each(item => {
				$(item).set('$$$$show', tempUnit === 'c');
			});
			$('.fahrenheit', tempUnitInput.$element).each(item => {
				$(item).set('$$$$show', tempUnit === 'f');
			});

			$('#temp_dial .markers').set('innerHTML', '');
			for (let i=0; i<12; i++) {
				let degree = i*scaleMultiplier;
				if (tempUnit === 'c') degree -= 15;
				const angle = degree*angleMultiplier;
				const item = HTML("<div class='label'></div>");
				if ((tempUnit === 'c' && (degree % 15 == 0 || degree == -5) && degree > -15) ||
					(tempUnit === 'f' && degree % 30 == 0)) {
					$(item).add(HTML(`<span class="label">${degree}°</span>`));
				}
				$('#temp_dial .markers').add(item);
				$(item).set({
					$position: 'absolute',
					$paddingTop: '0.1em',
					$height: `${dialDiameter}rem`,
					$width: '1px',
					$left: `calc(50% + 2px - ${markerThickness}px / 2)`,
					$transform: `rotate(${angle}deg)`,
					$borderTop: `${markerThickness}px solid currentcolor`,
					$opacity: '0.8',
					$fontSize: '0.75rem',
					$zIndex: '2'
				});
				$('.label', item).set({
					$display: 'inline-block',
					$margin: 'auto',
					$transform: `translateX(-50%) rotate(${-angle}deg)`
				});
			};
			$('#temp_dial .indicator.current').set('$transform', `rotate(${indicatorRotation + 30}deg)`);
			$('#temp_dial .indicator.range').set('$transform', `rotate(${indicatorRotation}deg)`);
		}
		
		tempUnitInput.on('change', function() {
			drawTempDial();
		});
		drawTempDial();
		

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
