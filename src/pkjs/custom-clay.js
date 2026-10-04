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

		$('#presetThemes')[0].classList.add(clayConfig.meta.userData.displayShape);

		// Add image for each option and wire up to preset data
		$('[data-theme-id]').each(function (item) {
			const themeId = item.getAttribute('data-theme-id');
			$(item).add(HTML('<img src="data:image/png;base64,'+clayConfig.meta.userData.screenshots[themeId]+'"/>'));
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
		const forceWeatherToggle = clayConfig.getItemByMessageKey('REQUEST_WEATHER');
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

			if (weatherProviderInput.get() === 'none') {
				weatherEnabledToggle.set(false);
				weatherElements.forEach(function (element) {
					element.disable();
				});
				$('.description', weatherProviderInput.$element).set({
					$opacity: 0.25
				});
			} else {
				weatherEnabledToggle.set(true);
				weatherElements.forEach(function (element) {
					element.enable();
				});
				$('.description', weatherProviderInput.$element).set({
					$opacity: 1
				});
			}

			if (weatherProviderInput.get() === 'open-meteo' || weatherProviderInput.get() === 'none') {
				clayConfig.getItemByMessageKey('WEATHER_API_KEY').hide();
			} else {
				clayConfig.getItemByMessageKey('WEATHER_API_KEY').show();
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
			'max-width: 120px;'+
			'width: calc(25% - 1em);'+
			'margin-left: 0.5em;'+
			'margin-right: 0.5em;'+
			'vertical-align: bottom;'+
			'padding: 0;'+
			'background: none;'+
			'font-size: .8rem;'+
			'line-height: 1em; '+
			'color: #a4a4a4; '+
		'}'+
		'#presetThemes.bw [data-theme-id] {'+
			'width: calc(33% - 1em);'+
		'}'+
		'#presetThemes [data-theme-id] img {'+
			'max-width: 100%;'+
			'display: block;'+
			'margin: auto;'+
			'margin-top: 0.3rem;'+
			'box-shadow: 0 0 0 2pt #414141;'+
			'border-radius: 50%;'+
			'transition: transform 100ms ease-in;'+
		'}'+
		'#presetThemes [data-theme-id]:active img {'+
			'transform: scale(0.95);'+
			'transition: none;'+
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
			'TIME_COLOUR': '000000',
			'DATE_COLOUR': 'AAAA00',
			'HOUR_HAND_COLOUR': 'FFFF55',
			'MINUTE_HAND_COLOUR': 'FFFF55',
			'HANDS_SHAPE': handShapes.pencil,
			'TICKS_COLOUR': 'AAAA00',
			'TICKS_SIZE': '1',
			'TEMP_NOW_COLOUR': 'FFFF00',
			'TEMP_RANGE_COLOUR': '555555'
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
