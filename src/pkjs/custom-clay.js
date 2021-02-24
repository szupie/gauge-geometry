module.exports = function(minified) {
	var clayConfig = this;
	var _ = minified._;
	var $ = minified.$;
	var HTML = minified.HTML;

	clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
		$('head').add(HTML('<style type="text/css" id="config-style"></style>'));

		clayConfig.getItemById('save-button').$element.set({
			$position: 'sticky',
			$bottom: '0',
			$background: 'linear-gradient(to bottom, rgba(51, 51, 51, 0) 33%, rgba(51, 51, 51) 66%)',
			$zIndex: 3
		});

		// Themes
		$('#config-style').add(
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
			'#presetThemes.rect [data-theme-id] img {'+
				'border-radius: 0;'+
			'}'
		);
		const themesAccordion = clayConfig.getItemById('themes-accordion').$element;
		themesAccordion[0].classList.add('accordion')
		function toggleThemeAccordion() {
			themesAccordion[0].classList.toggle('shown');
		}
		themesAccordion.on('click', toggleThemeAccordion);
		$(themesAccordion[0].nextSibling).on('click', toggleThemeAccordion);
		$('[data-theme-id]').each(function (item) {
			const themeId = item.getAttribute('data-theme-id');
			$(item).add(HTML('<img src="data:image/png;base64,'+screenshots[themeId]+'"/>'));
			$(item).on('click', function () {
				applyThemeSettings(themeSettings[themeId]);
			});
		});

		// Add notch marks to slider input to make sizing increments clearer
		const ticksSizeInput = $('input.slider', clayConfig.getItemByMessageKey('TICKS_SIZE').$element);
		const sliderCount = ticksSizeInput.get('@max');
		ticksSizeInput.set({
			$background: 'repeating-linear-gradient(to right, #666, #666 1px, transparent 1px, transparent calc(100% / '+sliderCount+' - 1px), #666 calc(100% / '+sliderCount+' - 1px), #666 calc(100% / '+sliderCount+'))',
			$backgroundSize: 'calc(100% - 1.4rem) 0.75rem',
			$backgroundPosition: 'center',
			$backgroundRepeat: 'no-repeat'
		});

		// Style subsection headings
		$('#config-style').add(
			'.component-heading:not(:first-child) {'+
				'padding-bottom: 0.5rem;'+
			'}'+
			'.component-heading:not(:first-child) h6 { '+
				'color: #a4a4a4; '+
				'text-transform: uppercase;'+
			'}'+
			':not(.component-heading) + .component-heading h6 {'+
				'margin-top: 2rem;'+
			'}'
		);

		const handsShapeSection = clayConfig.getItemById('hands-shape').$element;
		handsShapeSection.set('id', 'hands-shape');
		$('label .label', handsShapeSection).each(function (item) {
			item.innerHTML = '<svg width="180" height="20">'+
				handSVGs[item.textContent]+
				'</svg>';
		});
		$('#config-style').add(
			'#hands-shape .label > svg {'+
				"margin-left: -70px;"+
				"transform: scale(0.9);"+
				"opacity: 0.8;"+
			'}'
		);

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

		$('#config-style').add(
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
			'}'
		);

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


	// Data

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
			'TICKS_COLOUR': '0055FF',
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
		bw_light: {
			'BG_COLOUR': 'FFFFFF',
			'TIME_COLOUR': '000000',
			'DATE_COLOUR': '000000',
			'HOUR_HAND_COLOUR': '000000',
			'MINUTE_HAND_COLOUR': '000000',
			'TICKS_COLOUR': '000000',
			'TICKS_SIZE': '2',
			'TEMP_NOW_COLOUR': '000000',
			'TEMP_RANGE_COLOUR': 'AAAAAA'
		},
		bw_dark: {
			'BG_COLOUR': '000000',
			'TIME_COLOUR': 'FFFFFF',
			'DATE_COLOUR': 'FFFFFF',
			'HOUR_HAND_COLOUR': 'FFFFFF',
			'MINUTE_HAND_COLOUR': 'AAAAAA',
			'TICKS_COLOUR': 'FFFFFF',
			'TICKS_SIZE': '2',
			'TEMP_NOW_COLOUR': 'FFFFFF',
			'TEMP_RANGE_COLOUR': 'AAAAAA'
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
		nautical: "iVBORw0KGgoAAAANSUhEUgAAALQAAAC0BAMAAADP4xsBAAAAKlBMVEUAAAAATpjAwMBPiKYAXpsxYJ1+jKqrP0qBgYEwNmhyL0R0VHM7TmytVl2hHrm7AAAAAXRSTlMAQObYZgAAB+pJREFUeNq1m81u4zYQgO03MBVL6aa9WIV0N5zck9q9r5V12qvVzQP0ZKQ5LRYFct1DsY+w6AO079E3KsW/ETkkh5alQWLLsvRpNBqSMxxzlibMktloMmdIFrMxBHgjq240Hl1zG7Z7GE3xuUPe7R5GUhzIgB7HKK5tlzsuozxN5kWP4SnMjx7BC+VZ2NaXe/g8hL7YwQ3Z9euLG0+APEbDjHMuMbd0hxQiHJhujuUDo9jgkekmEWfwv7PQLNnQD+I/KPlTWZbP+85pjEemmAPQTSlkj8illOc+ekEr3UOXfnRRaql6aJagtLF1YdBYZ623tLVSmyZrD2n86LbsyQeFptmWu+rT99gcYJJ+A0hu4bkfrXb6vlyktvDMnI2Vrvi+J7nV//Ksbqlw0S3w8jPUZjQ6V2S4g1WK2vMEdNZXVPhQndK7sgR0Y+mZo0dxAdqw4EpHGj2PokHNyvm6oi3CEtCZq6VrbJY+IBbYAPzN2UGpzVLQrVQychtsKFr7Bxgf7fHag0bnWkfrYhVhEZaCLrxdbBUPHObJaOeARlsf1B6Ezgza2hVHszS0UTF2I47SaejGGDZm/sVI6FwekAfRbBga/DHcR4ngmUa3pn2ACHRmheCOPXY8QBiMzqte4mBbRMZL56JhX7s3aFdtGZcuY2i4eYzOKidJ69njMvRRHQHoxUjoqq1YEM2UrQehm8zsg/yvhxYeMgy9L+sweq49cpjWZbVCqeXCaeXnoeGAxnLI8dC86ZdhNLsAnXGlAQ1yOZp3Ts9lGD2/AN2WdeNFL4LoVRo6459bGm2fQqChr2YxtG8MXRE9H3yUL1j8aLpThaP1ndDo7sQyW2WE1hClRdFzhC45mhgbIUjNA+iFi+704H9xNHyzUiN6MpoloUsu3QERNEMG6d5j0RPkS3AAliFoSBmNm5yPvnkJRqoiGTWR6gCtv73YT0l8Mpmpia+HoG9Or4UvK8ghgy5DaGom8uvpNXdzGT0HYOcy2EUI9LvT6fQXysAyrTRkYDQaq83ZX+w00ZqsaLqtYejvT5oNSqopEch242geuHltlv+i2PAcM6k05OghNOuhpWtYLaB4PllsrrSZF4KZBb+LmHM4trthF11fW+ymantK57BJo2X3ZDpAfserk5AXZezMVbpm56D56fxFoctri106luYnJKJXJd8SOmfdCz+bq63YSk+tqXbCdHQH7zYy/i4VuwZ2oeYk+/N8yWipd/fBoPcnwy6lNM7sJI2WRIPm/xJ9o9mvam5SvCqlz0Mr/+D/qr/+dtKSuUrH0ctdH50pD+mjbwBtK32M1IZmaq4a0Nqvu2toNKj9L3ePImYOSDtmsWoGoN9p9D/dVaFWEEWbagbVuUr5ryyhwhEpqOxS0aD27/3WNw6afdVolobebLStaVGebflE2NZbjg7XuxybXku09UUcHanSgSeA2n+Izae4i2w4uUOHpee/oLbq/qJsznXRkaqRVPvvL4oMzTGIvougW1R+gZuJdiLLKBpXjZxLwhfJaLpqVLhfBNC3FpqqGoHS9YFvHUr/2Jh70ETVCD7UzjaNJqpGuOZTeC1yJdBrCx2vGsHdHHEFiEbTVSNzEYj5Amg7UqWrRoWjZusx9lag39toumqUeWpizqlvb58xmqgaaRQa4ByyZN9baKJqpN7rKJqDJdvOCqiqEULj7P+HNykduu8iVNVIZekx9K8afWfnjWTViNb6TclnB01VjWh0btC3NpqqGsU8BEz9Sai+ttFU1cj1a/z5Ow6Vr2t7qoWuGuHWKHdj9L2DpqtGeb8PgY8YvSDRaL6itede1E1gW4s5xHT0UcFqtxfDHvKnM89HlnbEJcRGKG/U6E/nosV3oOmTHn1wa1w707Vk1QjG42cZ9yn/wH3Ie0BjDb1oN3jCx6nGuBiEzqMRzpXsU1Wx4Ay0y67RcP646ci3s3PQDaBbQAei1LUuniSjXbLH/SX6vVNNSkQLcijC3snhHEo+UAOj0U9uOIlTjc3dAlfuaHReOuFk7UGv+5W7pfyn0Y2yMFj9iNK6zR5KmcloCDOBXWM0rpIm+HVjmxc6K0B3psYVaRpto+SlKriStPV6BiI9hEabUcVWGwJJid47P1nYPdA9nz2q4PnajUxGrR8tpPbXjffnfUe7Kd7NBqBFLBgOl7emA+lbJG1slJzgj3E2Uu4HoPeC7wmrINEwrRwkLQ4pouhHYw9XbTp62ouXUBCXG3vQaEyIorfGHo4kRaqwH6OXSuk18TtSf3wd1hoeIrQXECorMOjAY1Tk2xmWlFwm6CGg9N73U0E6A4O7wU1mA/bAQuaNe08xTe/awkP0qU1lu+huIGlfboxTe4XM0aELdScGHs1D9As9syD7azwUXG3gIfotQsyH6A1X6Vo9Q2iJWBJmcfoDuimXbpHSWG167qkx4y4EaQesNBZ6xqzQ23ChetNXmlZ7uWv983x8t55fPnTbH3e/xZQG6cUqW3/UmNm7f+xiGlA6qjaEQa0/sbDRHwF9m7awYMflJ6w0nlPtDtzqhkgIoH8O5FmNH71OWR6iF5SEqkatF52wYgaWwQSrRk+QgHUH6mdIi0ZHChn5QdzLQR5ozEGb5My1SVthjkkWVHXd3sM0i7WW3BxTLTF7vL18+VpgYVx++aK7UZbzTbkIccqlk1Mu+JxymeqUi2unXBI85ULmSZdfT7lofNKl7oMX6P8PRfNYfNjkUoYAAAAASUVORK5CYII=",
		purple: "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",
		rose: "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",
		gold: "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",
		salmon: "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",
		red: "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",
		blackgold: "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",
		bw_light: "iVBORw0KGgoAAAANSUhEUgAAAJAAAACoAQAAAAAnFFrVAAADtUlEQVR42sWWvY7UVhTHj+OJPAXCPMBKpotEBZpmC4ThIdznFVIlVPbQbDkgGpBQBlH6IYK3AaUJT7DC0Uhse6U0F7Bszsf99A1Suoy0Xvt3/3PO/Tjn74El+RA6pOhXuplrumpB81d6mCq6qva/oTTW9zMu/xtSpUcTlHwPEeIr1A5pRgqMmIPw/QhQrNAAkDs0MupKlflJ6JJu6wliNAM+tTHK8Lt1hKYcM8RIF5inCpD8qRgpxjGqUlRjiiJC47+jeYVaRPnyMUCDoLcpaldoySeJ9VEQjWWfbeV49A+hLlJdMzKxOkYHQsc/jx7tK0aXAXpSIzrt3weoo/K92n8I0NNlOcEfxxC9XpbLFRpStCF0ZdGe+mFLsU4hmm7ZLXRI3U7Q+NAiF37oErQXtG0dmvMETcWe0V8W0b8EjVWChjpBXZtFaKCHNZpSpLMA/cYFoPIld+gFo6GYPXrHqCsNos/fH6jkoHJISnd8APW6Vs+hdUj6R92FsKK/AO17ZlvhiHE+Q43dnFt0QKS7EnFh2+ri/tlj/TDDL5e2+S7Obz7WtzEbVLZFn99794u+BZXCeKaRn+0+7fQNKEZobbu/3F3v9E3IBnhrTeHV7vonXQL8nDnreHXv/YUuO9j+4Azm9/MS0QDbjbOh4/3qoMsRtltrVifZCQWbM2tpV4I0bGoyPi5yvLLrbVqxx0uLZtgs8ukYcYQfzc0jjGUG82Xi6G84I/dFsXQ8+cacEG5VPgOvuheE6VWrASpRGd8VXxWV8V3xVaOScZoQZEZlxwEXKirjuzj/dgZRWd+dMrJaVlnfFV9llfVd9tXavBtknAuqwq8gMr5rjHRiJA/4SEgjsr5rrFYJqlbI+q5BI6MxQmcWzQ49QGR8N0KDQ2qFeHYcy/quLIgzWkc1rxMVIFl8jGiLWl6j9V3zAps82pvX3Bwg+zIMkD3aPkCjlG8jiC+mTPoAKUSxylfOCcMwsmXyCC4twuyS36OZptpGKpw6LoBUJxte005UqPIZFe1XGc1rNO/bxi9opBMqojUOtZRVE6CWUKTq6OgiVdaZEu39ccgAqlYoVu2tyhdAFqkol0NNinpfci6Wr1WXMUWNL3KePa3Rt4Jbo28YtxO+rYZatrDxzed21beo23vfyHRCA52Qb3c6x64yKqlSDThmHUCqdGKbNiqp0tlUTh/YEEh9NYFZda6ivaX5irblr3FAVP5nKVjHXH/6BKWqvk9UTcOqQ/Cb3KiK4Jc7qtboTqr6Tqw04zdTTB+/UYyBcwAAAABJRU5ErkJggg==",
		bw_dark: "iVBORw0KGgoAAAANSUhEUgAAAJAAAACoAQAAAAAnFFrVAAADyUlEQVR42s2Wv2sUQRTHd3NJRtJsFYxwuCHlVbGRpLk7sbQJmMVKPAlqI5IyCN5uFa3s7NMZ18L/wKwECciRLmDnQv4ANxFxDcc+Z97Mmx87ESxdyGb3M9978+u972xw+bXto1fiFhbiziQJZ8W9U4p7lP0b8mPpHv+jK6rMcwfwBcBBeIdCI4YoAv5PB8HnGKBuoQHAVKMYUVpFTWCCCQRFB1wU8nfIXNTw3xYO6kx5Dy5iNe+ntJD8i1wUIXZR6aOCd1E7KL4chS2UcTQNli00kGjoo6yFgmkowbJE4rGZocwxCHMtdVRXEKlYKaINcR/dHBk03kPUt9ALMazueM1Cz8VIVsarFnrENcGtkY3uBEG/ha766L1AK4TGoh7uiVhdG818oBXUaO6BhxYOiOjwi6mHehL9yjQKNz00sz9GdMOguQ0PLex5aHHooV7WEKLtSII2msk9NLdpoZeYAAv7wVSjJ4gWN0KD1hH19jii6/qqSLlkqJFM3bjIs3aufk5M+sr6ib7kNpoDjk42qRRGPM48r/0o3ye0zRFLq2A+2aCy2jk83WUHTTCb71Hx7Ryd7bJvkIXJkEr06fH6a/YdyoU8o0J+Nrk2YT+gXkx0uT+eLE3YGTS9fEimsDVZ+soqgDeb2jq2jtd2WJXC+TttMA+PKo4G8POutqHRYbnNqhjO35JZdeVKRHB2myxtRSIGF5kwPkxy8r7fyh77hEK4UAv1kVCAvcnKW6GnOugUUtVVaFAHKQ4+0b4bT0PAWecqEPBTgwGUUqV8V/qqVCnflb6qVNSeAjRKRe3AJypV5Lsh3xGQKvLdTiPWHlXku9JXUUW+i75aqIKW7ZhQJf6EfFcZqdw4+cJfBWIcke8qqxVjE20tRL6rUIwodtApoVCjT5wo33XQQKOohXB0GIt8V04IeyRHVcdJZCE5eReJJcpwjuS76gDrGDRWx1xoIToMU4Noa3ODxAZXuI9GqtIkt1DEkasymdPlYRBRmtwP+hoBv0SuGhSKoWb8wSA+dD4BzGgKz8RKlDyW6TES61U544rVeZuYCcVih2pnjoNCplVioUwgR5WKrXNUTapSNDfbIRu4qoVc1ZhUJgEaRyX60ijxUW5STscyuap79FFikhxHL+ZoSkHP0RSMXglTVoNCLmFiik+vqilRvfamkMUODcQOmXIX+5iWSiWzlAFvIweQWdqBOtY+IbM0VJmTWzYEMr8Sy6xSymhjaSaj9Qcn4w1apXwXlGN6V+6jxNPkuadJULVtfZMrVW19uZ9wVQv9TeXH8nv8A5TvjQquooBkAAAAAElFTkSuQmCC"
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
