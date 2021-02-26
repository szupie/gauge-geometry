module.exports = function(minified) {
	var clayConfig = this;
	var _ = minified._;
	var $ = minified.$;
	var HTML = minified.HTML;

	clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {

		// Global CSS
		$('head').add(HTML('<style type="text/css" id="config-style"></style>'));
		$('#config-style').add(cssCode);


		// ==== Save button style ====
		clayConfig.getItemById('save-button').$element.set({
			$position: 'sticky',
			$bottom: '0',
			$background: 'linear-gradient(to bottom, rgba(51, 51, 51, 0) 33%, rgba(51, 51, 51) 66%)',
			$zIndex: 3
		});


		// ==== Themes section ====
		const themesAccordion = clayConfig.getItemById('themes-accordion').$element;

		// Accordionify
		themesAccordion[0].classList.add('accordion')
		function toggleThemeAccordion() {
			themesAccordion[0].classList.toggle('shown');
		}
		themesAccordion.on('click', toggleThemeAccordion);
		$(themesAccordion[0].nextSibling).on('click', toggleThemeAccordion);

		// Add image for each option and wire up to preset data
		$('[data-theme-id]').each(function (item) {
			const themeId = item.getAttribute('data-theme-id');
			$(item).add(HTML('<img src="data:image/png;base64,'+screenshots[themeId]+'"/>'));
			$(item).on('click', function () {
				applyThemeSettings(themeSettings[themeId]);
			});
		});
		

		// ==== Hands shape ====
		// Replace hand shape option text labels with images
		const handsShapeSection = clayConfig.getItemById('hands-shape').$element;
		handsShapeSection.set('id', 'hands-shape');
		$('label .label', handsShapeSection).each(function (item) {
			item.innerHTML = '<svg width="180" height="20">'+
				handSVGs[item.textContent]+
				'</svg>';
		});


		// ==== Ticks ====
		// Add notch marks to slider input to make sizing increments clearer
		const ticksSizeInput = $('input.slider', clayConfig.getItemByMessageKey('TICKS_SIZE').$element);
		const sliderCount = ticksSizeInput.get('@max');
		ticksSizeInput.set({
			$background: 'repeating-linear-gradient(to right, #666, #666 1px, transparent 1px, transparent calc(100% / '+sliderCount+' - 1px), #666 calc(100% / '+sliderCount+' - 1px), #666 calc(100% / '+sliderCount+'))',
			$backgroundSize: 'calc(100% - 1.4rem) 0.75rem',
			$backgroundPosition: 'center',
			$backgroundRepeat: 'no-repeat'
		});


		// ==== Weather ====

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

		const tempCurrentColourInput = clayConfig.getItemByMessageKey('TEMP_NOW_COLOUR');
		const tempRangeColourInput = clayConfig.getItemByMessageKey('TEMP_RANGE_COLOUR');

		$('.description', tempUnitInput.$element).set({
			$display: 'flex',
			$flexWrap: 'wrap',
			$justifyContent: 'center',
			$alignItems: 'center'
		});

		function numToHexColour(num) {
			return num.toString(16).padStart(6, "0");
		}
		function colourCorrect(colour) {
			return sunlightColorMap[colour];
		}
		function updateTempCurrentColour() {
			const currentColour = '#'+colourCorrect(numToHexColour(tempCurrentColourInput.get()));
			$('#temp_dial .indicator.current').set('$borderColor', currentColour);
		}
		function updateTempRangColour() {
			const rangeColour = '#'+colourCorrect(numToHexColour(tempRangeColourInput.get()));
			$('#temp_dial .indicator.range').set('$borderColor', rangeColour);
		}
		updateTempCurrentColour();
		updateTempRangColour();
		tempCurrentColourInput.on('change', updateTempCurrentColour);
		tempRangeColourInput.on('change', updateTempRangColour);

		function drawTempDial() {
			const tempUnit = tempUnitInput.get();
			var scaleMultiplier, angleMultiplier, indicatorRotation;
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
			$('.celsius', tempUnitInput.$element).each(function (item) {
				$(item).set('$$$$show', tempUnit === 'c');
			});
			$('.fahrenheit', tempUnitInput.$element).each(function (item) {
				$(item).set('$$$$show', tempUnit === 'f');
			});

			$('#temp_dial .markers').set('innerHTML', '');
			for (var i=0; i<12; i++) {
				var degree = i*scaleMultiplier;
				if (tempUnit === 'c') degree -= 15;
				const angle = degree*angleMultiplier;
				const item = HTML("<div class='tick'></div>");
				if ((tempUnit === 'c' && (degree % 15 == 0 || degree == -5) && degree > -15) ||
					(tempUnit === 'f' && degree % 30 == 0)) {
					$(item).add(HTML('<span class="label">'+degree+'°</span>'));
				}
				$('#temp_dial .markers').add(item);
				$(item).set('$transform', 'rotate('+angle+'deg)');
				$('.label', item).set('$transform', 'translateX(-50%) rotate('+(-angle)+'deg)');
			};
			$('#temp_dial .indicator.current').set('$transform', 'rotate('+(indicatorRotation + 30)+'deg)');
			$('#temp_dial .indicator.range').set('$transform', 'rotate('+indicatorRotation+'deg)');
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
				weatherElements.forEach(function (element) {
					element.disable();
				});
			} else {
				weatherEnabledToggle.set(true);
				weatherElements.forEach(function (element) {
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


	function applyThemeSettings(settings) {
		Object.keys(settings).forEach(function (key) {
			clayConfig.getItemByMessageKey(key).set(settings[key]);
		});
	};


	// ==== CSS ====
	const dialDiameter = 7;
	const markerThickness = 5;
	const currentIndicatorDiameter = 12;

	const cssCode =

		// subsection headings
		'.component-heading:not(:first-child) {'+
			'padding-bottom: 0.5rem;'+
		'}'+
		'.component-heading:not(:first-child) h6 { '+
			'color: #a4a4a4; '+
			'text-transform: uppercase;'+
		'}'+
		':not(.component-heading) + .component-heading h6 {'+
			'margin-top: 2rem;'+
		'}'+

		// themes section
		'#presetThemes {'+
			'text-align: center;'+
			'padding-top: 0.7rem;'+
		'}'+
		'.accordion:not(.shown) + .component {'+
			'max-height: 3rem;'+
		'}'+
		'.accordion + .component::before {'+
			'content: "";'+
			'position: absolute;'+
			'top: 0;'+
			'bottom: 0;'+
			'left: 0;'+
			'right: 0;'+
			'background: linear-gradient(rgba(72, 72, 72, 0), rgba(72, 72, 72, 1));'+
			'transition: opacity ease-out 200ms;'+
		'}'+
		'.accordion.shown + .component::before {'+
			'pointer-events: none;'+
			'opacity: 0;'+
		'}'+
		'.accordion + .component {'+
			'overflow: hidden;'+
			'max-height: 100vh;'+
			'transition-property: max-height, padding-top, padding-bottom;'+
			'transition-timing-function: ease-out;'+
			'transition-duration: 200ms;'+
		'}'+
		'.accordion-label::after {'+
			'content: "▾";'+
			'margin-left: 0.5rem;'+
			'display: inline-block;'+
			'transition: transform ease-out 200ms;'+
		'}'+
		'.accordion.shown .accordion-label::after {'+
			'transform: translateY(-10%) rotate(-180deg);'+
		'}'+
		'#presetThemes [data-theme-id] {'+
			'min-width: 0;'+
			'width: 5em;'+
			'margin-left: 0.5em;'+
			'margin-right: 0.5em;'+
			'vertical-align: bottom;'+
			'padding: 0;'+
			'background: none;'+
			'font-size: .8rem;'+
			'line-height: 1em; '+
			'color: #a4a4a4; '+
		'}'+
		'#presetThemes [data-theme-id] img {'+
			'max-width: 100%;'+
			'margin-top: 0.3rem;	'+
			'box-shadow: 0 0 0 2pt #414141;'+
			'border-radius: 50%;'+
		'}'+
		'#presetThemes.rect [data-theme-id] {'+
			'width: 6em;'+
		'}'+
		'#presetThemes.rect [data-theme-id] img {'+
			'border-radius: 0;'+
		'}'+

		// hands shape
		'#hands-shape .label > svg {'+
			"margin-left: -70px;"+
			"transform: scale(0.9);"+
			"opacity: 0.8;"+
		'}'+

		// temperature preview
		'#temp_dial {'+
			'height: '+dialDiameter+'rem;'+
			'width: '+dialDiameter+'rem;'+
			'background-color: #666;'+
			'border-radius: 100%;'+
			'position: relative;'+
		'}'+
		'#temp_dial .indicator.current {'+
			'height: '+currentIndicatorDiameter+'px;'+
			'width: '+currentIndicatorDiameter+'px;'+
			'background-color: #666;'+
			'border: 4px solid transparent;'+
			'border-radius: 100%;'+
			'position: absolute;'+
			'left: calc(50% - '+currentIndicatorDiameter/2+'px);'+
			'margin-top: -2px;'+
			'transform-origin: '+currentIndicatorDiameter/2+'px calc('+dialDiameter/2+'rem + 2px);'+
			'z-index: 1;'+
		'}'+
		'#temp_dial .indicator.range {'+
			'height: '+dialDiameter+'rem;'+
			'width: '+dialDiameter+'rem;'+
			'border-width: 6px;'+
			'border-style: solid;'+
			'border-radius: 100%;'+
			'position: absolute;'+
			'clip-path: polygon(50% 0, 50% 50%, 136% 0);'+
		'}'+
		'#temp_dial .markers .tick {'+
			'position: absolute;'+
			'padding-top: 0.1em;'+
			'height: '+dialDiameter+'rem;'+
			'width: 1px;'+
			'left: calc(50% + 2px - '+markerThickness+'px / 2);'+
			'border-top: '+markerThickness+'px solid currentcolor;'+
			'opacity: 0.8;'+
			'font-size: 0.75rem;'+
			'z-index: 2;'+
		'}'+
		'#temp_dial .markers .tick .label {'+
			'display: inline-block;'+
			'margin: auto;'+
		'}'+
		'#temp_sample {'+
			'margin-left: 0.75rem;'+
			'font-size: 0.75rem;'+
			'line-height: 1.5em;'+
			'opacity: 0.6;'+
		'}'+
		'#temp_sample h6 {'+
			'text-transform: uppercase;'+
			'line-height: inherit;'+
		'}'+
		'#temp_explain {'+
			'flex: 1 0 100%;'+
			'margin-top: 1rem;'+
		'}';


	// ==== Data ====

	const handShapes = Object.freeze({
		dauphine: '0',
		pencil: '1',
		baguette: '2',
		breguet: '3',
		swiss_rail: '4',
	});

	// Preset colour schemes
	const themeSettings = {
		classic: {
			'BG_COLOUR': 'FFFFFF',
			'TIME_COLOUR': '000000',
			'DATE_COLOUR': '000055',
			'HOUR_HAND_COLOUR': 'AA0000',
			'MINUTE_HAND_COLOUR': '0055FF',
			'HANDS_SHAPE': handShapes.dauphine,
			'TICKS_COLOUR': '000055',
			'TICKS_SIZE': '2',
			'TEMP_NOW_COLOUR': 'AA0000',
			'TEMP_RANGE_COLOUR': 'FF55AA'
		},
		nautical: {
			'BG_COLOUR': '0000FF',
			'TIME_COLOUR': 'FFFFFF',
			'DATE_COLOUR': 'FFFFFF',
			'HOUR_HAND_COLOUR': 'FF0000',
			'MINUTE_HAND_COLOUR': 'FFFFFF',
			'HANDS_SHAPE': handShapes.dauphine,
			'TICKS_COLOUR': '5555FF',
			'TICKS_SIZE': '4',
			'TEMP_NOW_COLOUR': 'AAAAAA',
			'TEMP_RANGE_COLOUR': '55AAFF'
		},
		gold: {
			'BG_COLOUR': 'FFFF00',
			'TIME_COLOUR': '555500',
			'DATE_COLOUR': '555500',
			'HOUR_HAND_COLOUR': '0055AA',
			'MINUTE_HAND_COLOUR': '005555',
			'HANDS_SHAPE': handShapes.pencil,
			'TICKS_COLOUR': '555500',
			'TICKS_SIZE': '2',
			'TEMP_NOW_COLOUR': '0055AA',
			'TEMP_RANGE_COLOUR': '555500'
		},
		rose: {
			'BG_COLOUR': 'FFFFFF',
			'TIME_COLOUR': 'FF0055',
			'DATE_COLOUR': '0055AA',
			'HOUR_HAND_COLOUR': '0055AA',
			'MINUTE_HAND_COLOUR': '0055AA',
			'HANDS_SHAPE': handShapes.pencil,
			'TICKS_COLOUR': '0055AA',
			'TICKS_SIZE': '2',
			'TEMP_NOW_COLOUR': 'FF0055',
			'TEMP_RANGE_COLOUR': '0055AA'
		},
		purple: {
			'BG_COLOUR': '550055',
			'TIME_COLOUR': 'FFFFFF',
			'DATE_COLOUR': 'FFAAFF',
			'HOUR_HAND_COLOUR': '00AAFF',
			'MINUTE_HAND_COLOUR': 'FF00AA',
			'HANDS_SHAPE': handShapes.baguette,
			'TICKS_COLOUR': 'FFAAFF',
			'TICKS_SIZE': '3',
			'TEMP_NOW_COLOUR': '5555FF',
			'TEMP_RANGE_COLOUR': 'FF55FF'
		},
		salmon: {
			'BG_COLOUR': 'FFFFFF',
			'TIME_COLOUR': '0000AA',
			'DATE_COLOUR': '0000AA',
			'HOUR_HAND_COLOUR': '0000FF',
			'MINUTE_HAND_COLOUR': '0055FF',
			'HANDS_SHAPE': handShapes.breguet,
			'TICKS_COLOUR': 'FF0055',
			'TICKS_SIZE': '2',
			'TEMP_NOW_COLOUR': 'FF0000',
			'TEMP_RANGE_COLOUR': 'FF5500'
		},
		red: {
			'BG_COLOUR': 'FF0000',
			'TIME_COLOUR': 'FFAAAA',
			'DATE_COLOUR': 'FFFFFF',
			'HOUR_HAND_COLOUR': 'FFFFFF',
			'MINUTE_HAND_COLOUR': 'FFFFFF',
			'HANDS_SHAPE': handShapes.swiss_rail,
			'TICKS_COLOUR': 'FF5500',
			'TICKS_SIZE': '4',
			'TEMP_NOW_COLOUR': 'FFAAAA',
			'TEMP_RANGE_COLOUR': '550000'
		},
		blackgold: {
			'BG_COLOUR': '000000',
			'TIME_COLOUR': 'AAAA00',
			'DATE_COLOUR': 'AAAA00',
			'HOUR_HAND_COLOUR': 'FFFF55',
			'MINUTE_HAND_COLOUR': 'FFFF55',
			'HANDS_SHAPE': handShapes.pencil,
			'TICKS_COLOUR': 'AAAA00',
			'TICKS_SIZE': '1',
			'TEMP_NOW_COLOUR': 'FFFF00',
			'TEMP_RANGE_COLOUR': 'AAAAAA'
		},
		bw_classic: {
			'BG_COLOUR': 'FFFFFF',
			'TIME_COLOUR': '000000',
			'DATE_COLOUR': '000000',
			'HOUR_HAND_COLOUR': '000000',
			'MINUTE_HAND_COLOUR': 'AAAAAA',
			'HANDS_SHAPE': handShapes.dauphine,
			'TICKS_COLOUR': '000000',
			'TICKS_SIZE': '2',
			'TEMP_NOW_COLOUR': '000000',
			'TEMP_RANGE_COLOUR': 'AAAAAA'
		},
		bw_pointer: {
			'BG_COLOUR': '000000',
			'TIME_COLOUR': 'FFFFFF',
			'DATE_COLOUR': 'FFFFFF',
			'HOUR_HAND_COLOUR': 'FFFFFF',
			'MINUTE_HAND_COLOUR': 'FFFFFF',
			'HANDS_SHAPE': handShapes.pencil,
			'TICKS_COLOUR': 'FFFFFF',
			'TICKS_SIZE': '1',
			'TEMP_NOW_COLOUR': 'FFFFFF',
			'TEMP_RANGE_COLOUR': 'AAAAAA'
		},
		bw_bubbles: {
			'BG_COLOUR': '000000',
			'TIME_COLOUR': 'FFFFFF',
			'DATE_COLOUR': 'FFFFFF',
			'HOUR_HAND_COLOUR': 'AAAAAA',
			'MINUTE_HAND_COLOUR': 'AAAAAA',
			'HANDS_SHAPE': handShapes.swiss_rail,
			'TICKS_COLOUR': 'AAAAAA',
			'TICKS_SIZE': '4',
			'TEMP_NOW_COLOUR': 'AAAAAA',
			'TEMP_RANGE_COLOUR': 'FFFFFF'
		},
		bw_newsprint: {
			'BG_COLOUR': 'AAAAAA',
			'TIME_COLOUR': '000000',
			'DATE_COLOUR': '000000',
			'HOUR_HAND_COLOUR': 'FFFFFF',
			'MINUTE_HAND_COLOUR': 'FFFFFF',
			'HANDS_SHAPE': handShapes.baguette,
			'TICKS_COLOUR': 'FFFFFF',
			'TICKS_SIZE': '3',
			'TEMP_NOW_COLOUR': 'FFFFFF',
			'TEMP_RANGE_COLOUR': '000000'
		},
		bw_woodcut: {
			'BG_COLOUR': 'AAAAAA',
			'TIME_COLOUR': 'FFFFFF',
			'DATE_COLOUR': 'FFFFFF',
			'HOUR_HAND_COLOUR': 'FFFFFF',
			'MINUTE_HAND_COLOUR': 'FFFFFF',
			'HANDS_SHAPE': handShapes.breguet,
			'TICKS_COLOUR': '000000',
			'TICKS_SIZE': '2',
			'TEMP_NOW_COLOUR': '000000',
			'TEMP_RANGE_COLOUR': 'FFFFFF'
		}
	};

	const handSVGs = {
		'Dauphine': '<path fill-rule="evenodd" clip-rule="evenodd" d="M84 10l6-6 80 6-80 6-6-6z" fill="#fff"/></svg>',
		'Pencil': '<path fill-rule="evenodd" clip-rule="evenodd" d="M153 7l10 3-10 3H87V7h66z" fill="#fff"/><circle cx="90" cy="10" r="6" fill="#fff"/>',
		'Baguette': '<path d="M160 10H90" stroke="#fff" stroke-width="5" stroke-linecap="round"/>',
		'Breguet': '<path d="M175 10H90" stroke="#fff" stroke-width="5" stroke-linecap="round"/><circle cx="90" cy="10" r="5" fill="#fff"/><circle cx="155" cy="10" r="6" fill="#fff"/><circle cx="156.5" cy="10" r="3.5" fill="#484848"/>',
		'Swiss Rail': '<path d="M160 10H70" stroke="#fff" stroke-width="3" stroke-linecap="round"/><circle cx="90" cy="10" r="3" fill="#fff"/><circle cx="160" cy="10" r="6" fill="#fff"/>'
	}


	const screenshots = {
		classic: "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",
		nautical: "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",
		gold: "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",
		rose: "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",
		purple: "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",
		salmon: "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",
		red: "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",
		blackgold: "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",
		bw_classic: "iVBORw0KGgoAAAANSUhEUgAAAJAAAACoAQAAAAAnFFrVAAADwklEQVR4AcXWAYckRxQH8Dcml4lItgOBODoCiQHOwIWVvgUgOKQsYD9DEMlBujrCBZhEHAmROQHRi/0GuV7kVoIcCGBdx5DjIL3hqLu0eumqf1d11dQGAdfsmvn1U11d/eo/TZwchtYpfWw+6ML8VyD9j/nS5+Z/V/4PSsa6/Ir83KjLJurJfiHapZ6o8KQsdUSZp85+bokWO9QQzT21lmTWzTyxMkRFTzFpYqYyphmzLCLq58xNTGrB3OYB4a+LqQNHlKdUMPeLiNrLSS/4r5DKgeb8MKAGVCSkyx3ieY/hH4LMudkFaB3Q1pCMqs5AGEtaWhva/LKZSOaWTgOqioG21VlAn5UDnVcPAvqceUs/bUL6gvl0h75M6R1D546qQfqbZqxtSM/uBqsKenIjoV9lQBj+DiW0RNVLpSctElLHlaXfJnpymNDjk4TuHCW0vDeLqGEtyl3qa96lCxHQJ4bKx8c89/SNpd8P9UT3LS1PRjLHHw+Y20/FkSe0brtf39vt1XdF6Qn7p3ur5oCe0UCvC7cVNgXzUyq4Wx47Wg+kZMZKHLptdXv/6i11Y86qPuF2pOt7t9SbVD4VR26Lfn3t/ofqNcr/rjcsQV+t/lypV2jxsyjcdv929Wil9mhW1z4Uvls9eltlRB99YKIDdO3stsokvfGjD5jvr2cDNfTqTR9Dm/18rbKWXr7rwmqLlejoyvsu0s5BivY2CL6hyRnLQ1cKxOOpI00vlvgkHTG9wDgOzFg4Oefejv4DbUdq5izt5AW53G0zTfauaxARX7Aiyn0Vche5iirkLnIVVe68JJr5KpynrCNUudzVVGpClcvdfmajFlXIXeQqqsbcVQsk3YEhnFcZopYMIXfHIO0t4UuXW1KWkLuIWkd5QJ0l5G5EbUjtVUfa03uGkLsRNZ66HcJUMRZyFzeEK46JqhYJjYujQjJLVI4kLWEhuZ+ocj9zIUl09MFE7tHWAbVoX+FJujapA+pAYZXvnC1zBZJEuOKpJyJMaSJtplpGVWbqVJiqrRtemZXIw9nLzqxXFs2rxe9tMK+qzdEw9URNgbYSAZWgoEoOpKOqmUSLoqrxxAOJlIKqKqxCA8ymKpD2JFKqp5aLxkKv+ismhKrOk3b3iMaN7hEbJloJbKumwBKKafNFq4oujdYeG9k8ocY9IXSpmuOtQEyhoIg1uQRAl/a0aH1OoEv12Dl1EEOE/hJBWMmgoxFpUUe7BEEg1yDkLoWJGR51QmlVXSdVQtiqdfBOPlYtgjd3VEW0TKv+a6zkiv8CUxMtO1expbQAAAAASUVORK5CYII=",
		bw_pointer: "iVBORw0KGgoAAAANSUhEUgAAAJAAAACoAQAAAAAnFFrVAAADZElEQVR4AczVxaIkNRQG4IQsgmeJU/MGuEs9Re9w26ArnE6hO+Q5+iGY4DayZTeFL8mgV+rmUMlfSZ104V7Xv/7bkpP/ir9+dfHbKYJd0sXv+jdS/+PL+Pl3RekPom1SRK6QTmSIfCGTfm+IdraoJRoKNYmsN6GQ0JHIKapJ0qhdTUEI6ypSgxBtTXpHiKZnhC9TkwFX1C/JCaF2Kmp+nuQWdSMNFbW/m0SmQ6D4a5joEUZnxu+2Sl0LEolsovSO7rj6jpme6RPdzOixROtrGT3sEl3K6PYu3ZGT7cSpFaVdu7AmSYKn1iNhuyvSZQIKmSU1XzPCw7cnFkSH058/doUk2W1SYZ3ospn0sCCzs6DGL6jtQ6Zrp+2wXaFmIhKFzHuJVJhJ70bq9CCGTGpIZHZkIUmXRmp8mZyRXosj1/YzCepHctaxWW19JOoYNYMQ5o1QTXQY6ashH4U73EjUCUM7mR5J5IQ+4fOxevytz16KraAP96KZ6L2TI+0IZV0+og8cu+4VTSGNiAU9dOS8I/pb4sf9viPnHtEnyamDUgp3Hzn3Y+1tr/dRHZGOXfu49u3+Vz+WgrnzPT+SoX0qNXTHW/0j2ms6OE7kecFIOnieyHESFHjxYcnCVI+ncsL1+pJu5aTSo28KNUHYFFyV3m0GSdTNKaJRNVE/pdC7qkevIoXezb2KFG63RKGkcDt5Q0jl3pXUSUIq967CjiCFjUCvTin0bupVN51x3K49qpYioXenIlWJ8IfpE+lE6F1UbaaekUmE3q2o4dR8lkkWeiMSereitpDZIrxUPBZ6F29oekY0qt5Z0LQ4mlNIVQiyiaaJVjOt8785ThYTbRlNW7th1BB57GOJTmOyYWRAPMUnZw3CmIypUwsREWbiwkySnKIuTnRJKYrzUj2jjivRV89o4nr5KhX/NJ6n1k2PgdnM1DqM1YpRB2IpO5KsUsFiRJFqC4mAFKc6teYpDECYUyBZaLWkzTxy1WNhVvkz1oSUKSTze8TgVu8RB6ZaCRyr1mEJV/Phq1YVU1qtPQ5y3KE27xCmNO6jzfuIKdUkZNltTKminab0BKZUlsmZa4im+WJlZeeJRqWJaqIFelcTuZKaepd4Y/Jrs5BlarPZLDKrX0od+mlILXeoqgKk9j1uVYRtBADQy5E0EKexsgAAAABJRU5ErkJggg==",
		bw_bubbles: "iVBORw0KGgoAAAANSUhEUgAAAJAAAACoAQAAAAAnFFrVAAAD5klEQVR4Ab3WAaTkRhgH8G+yXamFoLtcQerAWnCFukJ3H61SaOhLAW6rFFX1AAc2KRygigLQKHQvUADAhcNxngMeBYT2oIW8Uu/h2ek380++mXnps0CHm8v75TOZmXzzbYjbO2RaabqIbFNvmH6amX6xIdeiyvSz0kkYFUX2/t70q0pijo01fuJoXv9rSzp3PdH2D60Dsr1uhGJLiUYwBrHXqdbXt2it9Y1QaqnokgO5wQzpZqJDUpq1DOlAVDQBTXjsdUgxzyBtPcK/JKQEHFA7poYfcR1Q+t+kblHJdBPQ+g66rm4T/ZP1OQEyl3/uQWeOXp+avvCjXtYgYsL/dJGbfvve1tEnJmq+/cCjPKc53d89DKNWdLJ74NFHp5RT2Qoh+2se3qdFNopaVhx1MkTtDH1OHHU/oF+JnzjnCyH1iACOomeE5ta4KEa0BF2VjrIxVTtL7zpSmxFFpSMFmpKj1ZmlGR2E8gf2dcw9qn+wtHSk8o2lzFFUVzYBKroZaJpnhtRGgVxUVEoyISotpySEqLSZebmKqGYOclFJs/RImaikzYajsG149numv8uBzpjyBVH8Wo7V4+e/P1l9zREv+RE9vbh8oi43NPlRjug3r97/Pu4qUks5yN+ev30ed5zJn8lx/+r8HtOK6BcpCl+e3/st7nh9n5rSAXr18HHcRVX0kxSYL150TCqbShl6c/u8PeNZ72e2WDHQSZ8miw/7kpZL5nAuo/Ct/EqO8sgZXUolR6vDeo8ot7M0aRB1IjtLhZ38BU8DdXc1+07bVf+FEK0pUx9r3ZprK2aSFT3FgkC8FN7WUywbxAvm17PS+gBC3eXX80eXaBDq7ktSl6VypBt+PZMD9t6Sufm0HOoqiO+/1di6ynRhyNyPG1vp2n4nUHf7QqpyV1WT1lJko1B3+1I7zS21Hs0QZYtsEJX6NPt5IDXQwm4z6u4QVRpaCyWIEsJUMZYhusGC8ERU1AOW1c8LhM0JoswWFmW/xsJS/wOmcqHd8DNXOxp+DC88WsurFUqRviB08ZAmc6EEdEUrof4o1CY1d6BCazyxLgfS3MIoZaZqapmLMlPXTR+F4WOzEy2P5Z6YmP3qOMrNK+1/b6/cgtIWCaMdrRuklU/liAomFdChQIqC1kJ0lHZCkgCH26SOElLOjSW5WhyhREgWhMR1a5QDIzvhjtW68bYQWRrsKrK033sQPqDMG1oL2Sw177Foe0KWxprvNT0hSyf6OpU6gSxVfeaAkGzaL0Oou4VfrFB3XUbLB2fMNxyh7tqhQK7dTeF3NOamTr2v7cIS7b1v8mfjqEeWotr7vr8j6shYeOK/gT6LHay1W8sAAAAASUVORK5CYII=",
		bw_newsprint: "iVBORw0KGgoAAAANSUhEUgAAAJAAAACoAQAAAAAnFFrVAAAEGUlEQVR4AbXWAYQcVxwG8P9suhKrEaFL1bGSomdAOKXgdkoDtO2cm+fAOgU0AKyAdgbkatGgDe3BjZbeyzt6AMDlIEFAYRXgHIEL5MKxuW729b3/9+bNm0lOKX2yzzE/7828/fbLkHpjkBBiLOyY2mnDfLzaO7HzPhTGxpmdN6es9v7g6+d2fnRykWqvdfGO7fsKx/+oyl6tMuryZaqVpB7PFHuVskrJYV6EVUnUaamEKGqpolcSFDOrqJ+RV0paReZf3FBZZGFDSbNdESp8kpbq4SlDhU+tIGRLmXWyloqxa6CStyvVUgJPEKiC1a9tJaIlqAmU5bQK1a0V5VYVajtQOjaKpliLV72krdr5Plhra2HX6n/ar9W1ufnjoNgK1tqZqQO6G30RqOGpyOn5ziRQ+bHSZq1AZfqxVduB2tPKqLuVimzsF8Kog1DdnyuzoxiPxl4NZsF5QQ1f1KeKHbP8iNXfJ15JfcjqSq3WdMHqyh2vdjWx+lKI2059NY9YTe6oT5zamRWsPhfiO6eGp17pZaj8mJzK9J9GJfYcanXE6qGeVErkM1brWng1nHMm9ueqU6mBZjV4lXm1pSdWDV86NZqaRZ4ZleXP6nxJ/dKkcFk/rbOa5a+Nuq4fBFkdLGJVfqhjp8ZjcwT6gUivLUQKtW0y+pt+qsrBrPp1XP347K81fSZSk4e0z+re8vvfSH2upMlDCXX51rtr6XDBeUigfrzx5Gs50LE5h+pXe33ls5X0sp6u675wDfDLzd9vykv6ZHcRS9cT763c/ih9R7/anHfTCOrnG0/uyV4+H74g6jh162o37Q50fuSbaXv5gx9k777Wh0Q9VqPx+OzbtLuu9U/EZTKq8vVQ65hIKU4hRpa7c7CJVhjDufsjr5S4dmomKOXG7rGSXJirNHIX1x4LVOES+d6NFXq1KlxCr/JxsULvul6FQu+6XoXC9YKIvMJ16qYEhd5FqXqF3s0s9IqJ61Uo9G7VdKtW4XrVmpp8E7rW3NDkW7WE2nfKfowws1PoXcxOoXed2mSF3nXq0XOrkoaaVko1FXq3oQqvylCJyD0jdkSjyk5w91BZxGojVMSrQhEU8V+V4omQ6EAJQqJXA1X4b9urBImmQEkkulZUJ+fAK5ecJcrNMlBEhB21V4Tk5LXK3P/Iea0kwgNFrNLIZaLescR5qaX6vhL+zht3b4XsNJ6Rv82ooYr4DUWirQizU4lX4l9VFCpkgtoq+y8KWW3siKzSBarjFFLaeEZkuXES+I0lbJ1CShunipQ2zl5xSksjip5TSGmKNysopNR921BIaUbdpO4JpJRancNbNZupB+tVximFdUrhIRpdiN7lpaDCARWOtoKh9tu2u9/z4J28gHodvLkfstpbBO/3hxep5lpv3fEf1MYEuNHZf6UAAAAASUVORK5CYII=",
		bw_woodcut: "iVBORw0KGgoAAAANSUhEUgAAAJAAAACoAQAAAAAnFFrVAAAEQklEQVR4Ab3WAWQkVxgH8FU96gCaBWhogYGCQku3wSpgc7Kvz4EBcUA5BKn2JlAqaOG2ABs5mr033NBCVduREsidHFyckmolIsxVr5vryma6O1/f+/5v3rzZFQf0uXsb5ud9b9/79m8aam40hBDXRG0Y9SY/DMw0gOreMM+6TTMvQ2EMrpt5K7hS+WtdWbE+oMT/o5K8UpJG5oOoUjHlPFM2o0ICZpWwSok/oEJ+HBFNnUr5MeVJ4ZSQRtGhJKhyRzFpOKv0cod1VehFs5qSulw4Et1K4X+mYqdYmE95CAXBn+m+pw55vZZTqBbnKlJ1JabdQnhKmb3dn1QKO5+8fjmrXpydz6qzxdMZpQ6SfVZrUIb/Jntakdqu1I/fBFpFol2pnz4SWv3xfsaKjPr5gdJq9c5qpVpvaLVdfO+ptDDqzm1PhVOxpNf61lM0VhuNhdW1SnVpaFTmqft0rJXw1cq4qdV2qQqtdl7wPbbb1e47Z95tQ/Ufe7cN1eqz2giUq0gtVhe9ShURqws1CKyKxwWrd3orPavkEOo7tfOLVckx1FpPDq0K95wab33CKt0s1UrR6bGKmmTVgN7KNFFxEZSqG93ltVamolSqr3Kjds5VqURHrJue6JyKaanuBZ8a1d+PodpNcUN8Zrqw1bPKHLmaaJVFQZy787o71epJIWSlTrRKnk2UUVirSxOtLi7NuRl1vK8UUaCS58cqgbr95NWFiD4X8vc9EULlD08epKRFuln+HtefDr/ufDUUMmqWv9o/D977cuuWLlkEiqA+Prp5JIdFU+iqBPXXox8eJWPaTC6z2CbAraObv8p/Wnudi1Foc+LZwbt5kvf/XnxMVECtPh2uy9GH/z7vu8zJHh6fJ/k9uni7TKal9sK1thwtUy6JDll9YfsrmijigtuNBYGxOMU3NB1tR3+iMHah8LvA2MVaUDEH5kljSWB0LkXEmyfX0TvnMSoSbohIrJxKopFVyN3BdeQqFHIXuVoqPCecBBSeE58XFHI3JvPPKuSuJPMHFHK3zFUo5C7nqla7RuF5mYcNo5C7Nlu7jYZLVc7VTA1YIXcxi2WozEjMaguKc9dXyF2nXiuVKNUHC6x4b6UKjIqcCusKu8NaVkkoUxFKsUoz7AspSHLK3ldxwSkw8JTAqeIkiJUidLRVPEW4oV1Ppe4enQqrnoAilRBlL1euc5aqikSm4kljQxeDIiLjdyvFncG37RR3RlZTfBIjs5aryOeV1/aFs/f3VaS4IV9FfOd1hU6bVaKuaE4RZqtSp9RLVeEr1KJZFdcVurC2FnqVZpWsqwTK3z16ufYd8f6S+gpdWjsvdGntVNGl/tljhax2Q+hStqVClyYmJzKr0KWSpqHLCXRpjM6xKkTmoL+gkvlkQu76vWoAbOaUzV0iAVUfUPUxr4jmFBGrTe+d3Kpr3pt7BPWK934fXalm15qv+B+SNwBHJSvOgwAAAABJRU5ErkJggg=="
	};


	const sunlightColorMap = {
		'000000': '000000', '000055': '001e41', '0000aa': '004387', '0000ff': '0068ca',
		'005500': '2b4a2c', '005555': '27514f', '0055aa': '16638d', '0055ff': '007dce',
		'00aa00': '5e9860', '00aa55': '5c9b72', '00aaaa': '57a5a2', '00aaff': '4cb4db',
		'00ff00': '8ee391', '00ff55': '8ee69e', '00ffaa': '8aebc0', '00ffff': '84f5f1',
		'550000': '4a161b', '550055': '482748', '5500aa': '40488a', '5500ff': '2f6bcc',
		'555500': '564e36', '555555': '545454', '5555aa': '4f6790', '5555ff': '4180d0',
		'55aa00': '759a64', '55aa55': '759d76', '55aaaa': '71a6a4', '55aaff': '69b5dd',
		'55ff00': '9ee594', '55ff55': '9de7a0', '55ffaa': '9becc2', '55ffff': '95f6f2',
		'aa0000': '99353f', 'aa0055': '983e5a', 'aa00aa': '955694', 'aa00ff': '8f74d2',
		'aa5500': '9d5b4d', 'aa5555': '9d6064', 'aa55aa': '9a7099', 'aa55ff': '9587d5',
		'aaaa00': 'afa072', 'aaaa55': 'aea382', 'aaaaaa': 'ababab', 'ffffff': 'ffffff',
		'aaaaff': 'a7bae2', 'aaff00': 'c9e89d', 'aaff55': 'c9eaa7', 'aaffaa': 'c7f0c8',
		'aaffff': 'c3f9f7', 'ff0000': 'e35462', 'ff0055': 'e25874', 'ff00aa': 'e16aa3',
		'ff00ff': 'de83dc', 'ff5500': 'e66e6b', 'ff5555': 'e6727c', 'ff55aa': 'e37fa7',
		'ff55ff': 'e194df', 'ffaa00': 'f1aa86', 'ffaa55': 'f1ad93', 'ffaaaa': 'efb5b8',
		'ffaaff': 'ecc3eb', 'ffff00': 'ffeeab', 'ffff55': 'fff1b5', 'ffffaa': 'fff6d3'
	};
	
};
