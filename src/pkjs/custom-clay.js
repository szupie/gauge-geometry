module.exports = function(minified) {
	var clayConfig = this;
	var _ = minified._;
	var $ = minified.$;
	var HTML = minified.HTML;

	clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
		$('head').add(HTML(`<style type="text/css" id="config-style"></style>`));

		clayConfig.getItemById('save-button').$element.set({
			$position: 'sticky',
			$bottom: '0',
			$backgroundColor: '#333333',
			$zIndex: 3
		});

		// Themes
		$('#config-style').add(`
			#presetThemes {
				text-align: center;
			}
			.accordion:not(.shown) + .component {
				max-height: 3rem;
			}
			.accordion + .component::before {
				content: '';
				position: absolute;
				top: 0;
				bottom: 0;
				left: 0;
				right: 0;
				background: linear-gradient(rgba(72, 72, 72, 0), rgba(72, 72, 72, 1));
				transition: opacity ease-out 200ms;
			}
			.accordion.shown + .component::before {
				pointer-events: none;
				opacity: 0;
			}
			.accordion + .component {
				overflow: hidden;
				max-height: 100vh;
				transition-property: max-height, padding-top, padding-bottom;
				transition-timing-function: ease-out;
				transition-duration: 200ms;
			}
			.accordion-label::after {
				content: '▾';
				margin-left: 0.5rem;
				display: inline-block;
				transition: transform ease-out 200ms;
			}
			.accordion.shown .accordion-label::after {
				transform: translateY(-10%) rotate(-180deg);
			}
			#presetThemes [data-theme-id] {
				min-width: 0;
				width: 5em;
				margin-left: 0.5em;
				margin-right: 0.5em;
				padding: 0;
				background: none;
				font-size: .8rem;
				color: #a4a4a4; 
			}
			#presetThemes [data-theme-id] img {
				max-width: 100%;
				margin-top: 0.2rem;	
				box-shadow: 0 0 0 2pt #414141;
				border-radius: 50%;
			}
		`);
		const themesAccordion = clayConfig.getItemById('themes-accordion').$element;
		themesAccordion[0].classList.add('accordion')
		function toggleThemeAccordion() {
			themesAccordion[0].classList.toggle('shown');
		}
		themesAccordion.on('click', toggleThemeAccordion);
		$(themesAccordion[0].nextSibling).on('click', toggleThemeAccordion);
		$('[data-theme-id]').each(item => {
			const themeId = item.getAttribute('data-theme-id');
			$(item).add(HTML(`<img src='${screenshots[themeId]}'/>`));
			$(item).on('click', ()=>{
				applyThemeSettings(themeSettings[themeId]);
			});
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
		$('#config-style').add(`
			.component-heading:not(:first-child) {
				padding-bottom: 0.5rem;
			}
			.component-heading:not(:first-child) h6 { 
				color: #a4a4a4; 
				text-transform: uppercase;
			}
			:not(.component-heading) + .component-heading h6 {
				margin-top: 2rem;
			}
		`);

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

		$('#config-style').add(`
			#temp_dial {
				height: ${dialDiameter}rem;
				width: ${dialDiameter}rem;
				background-color: #666;
				border-radius: 100%;
				position: relative;
			}
			#temp_dial .indicator.current {
				height: ${currentIndicatorDiameter}px;
				width: ${currentIndicatorDiameter}px;
				background-color: #666;
				border: 4px solid transparent;
				border-radius: 100%;
				position: absolute;
				left: calc(50% - ${currentIndicatorDiameter/2}px);
				margin-top: -2px;
				transform-origin: ${currentIndicatorDiameter/2}px calc(${dialDiameter/2}rem + 2px);
				z-index: 1;
			}
			#temp_dial .indicator.range {
				height: ${dialDiameter}rem;
				width: ${dialDiameter}rem;
				border-width: 6px;
				border-style: solid;
				border-radius: 100%;
				position: absolute;
				clip-path: polygon(50% 0, 50% 50%, 136% 0);
			}
			#temp_dial .markers .tick {
				position: absolute;
				padding-top: 0.1em;
				height: ${dialDiameter}rem;
				width: 1px;
				left: calc(50% + 2px - ${markerThickness}px / 2);
				border-top: ${markerThickness}px solid currentcolor;
				opacity: 0.8;
				font-size: 0.75rem;
				z-index: 2;
			}
			#temp_dial .markers .tick .label {
				display: inline-block;
				margin: auto;
			}
			#temp_sample {
				margin-left: 0.75rem;
				font-size: 0.75rem;
				line-height: 1.5em;
				opacity: 0.6;
			}
			#temp_sample h6 {
				text-transform: uppercase;
				line-height: inherit;
			}
			#temp_explain {
				flex: 1 0 100%;
				margin-top: 1rem;
			}
		`);

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
				const item = HTML("<div class='tick'></div>");
				if ((tempUnit === 'c' && (degree % 15 == 0 || degree == -5) && degree > -15) ||
					(tempUnit === 'f' && degree % 30 == 0)) {
					$(item).add(HTML(`<span class="label">${degree}°</span>`));
				}
				$('#temp_dial .markers').add(item);
				$(item).set('$transform', `rotate(${angle}deg)`);
				$('.label', item).set('$transform', `translateX(-50%) rotate(${-angle}deg)`);
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


	function applyThemeSettings(settings) {
		for (const key in settings) {
			clayConfig.getItemByMessageKey(key).set(settings[key]);
		}
	};

	// Preset colour schemes
	const themeSettings = {
		classic: {
			'HOUR_HAND_COLOUR': 'AA0000',
			'MINUTE_HAND_COLOUR': '0055FF',
			'TIME_COLOUR': '000000',
			'DATE_COLOUR': '000055',
			'BG_COLOUR': 'FFFFFF',
			'TICKS_COLOUR': '000055',
			'TICKS_SIZE': '2',
			'TEMP_NOW_COLOUR': 'AA0000',
			'TEMP_RANGE_COLOUR': 'FF55AA'
		},
		nautical: {
			'HOUR_HAND_COLOUR': 'FF0000',
			'MINUTE_HAND_COLOUR': 'FFFFFF',
			'TIME_COLOUR': 'FFFFFF',
			'DATE_COLOUR': 'FFFFFF',
			'BG_COLOUR': '0000FF',
			'TICKS_COLOUR': 'FFFFFF',
			'TICKS_SIZE': '1',
			'TEMP_NOW_COLOUR': 'AAAAAA',
			'TEMP_RANGE_COLOUR': '55AAFF'
		},
		gold: {
			'HOUR_HAND_COLOUR': '0055AA',
			'MINUTE_HAND_COLOUR': '555555',
			'TIME_COLOUR': '555500',
			'DATE_COLOUR': '555500',
			'BG_COLOUR': 'FFFF00',
			'TICKS_COLOUR': 'AAAA00',
			'TICKS_SIZE': '2',
			'TEMP_NOW_COLOUR': '0055AA',
			'TEMP_RANGE_COLOUR': '555500'
		},
		rose: {
			'HOUR_HAND_COLOUR': '0055AA',
			'MINUTE_HAND_COLOUR': '0055AA',
			'TIME_COLOUR': 'FF0055',
			'DATE_COLOUR': '0055AA',
			'BG_COLOUR': 'FFFFFF',
			'TICKS_COLOUR': '0055AA',
			'TICKS_SIZE': '2',
			'TEMP_NOW_COLOUR': 'FF0055',
			'TEMP_RANGE_COLOUR': '0055AA'
		},
		purple: {
			'HOUR_HAND_COLOUR': '0055FF',
			'MINUTE_HAND_COLOUR': 'FF00AA',
			'TIME_COLOUR': 'FFFFFF',
			'DATE_COLOUR': 'FFAAFF',
			'BG_COLOUR': '550055',
			'TICKS_COLOUR': 'FFAAFF',
			'TICKS_SIZE': '3',
			'TEMP_NOW_COLOUR': '5555FF',
			'TEMP_RANGE_COLOUR': 'FF55FF'
		},
		red: {
			'HOUR_HAND_COLOUR': '000000',
			'MINUTE_HAND_COLOUR': '550000',
			'TIME_COLOUR': 'FFFFFF',
			'DATE_COLOUR': 'FFFFFF',
			'BG_COLOUR': 'FF0000',
			'TICKS_COLOUR': 'FFFFFF',
			'TICKS_SIZE': '2',
			'TEMP_NOW_COLOUR': 'FFAAFF',
			'TEMP_RANGE_COLOUR': '550000'
		},
		black: {
			'HOUR_HAND_COLOUR': 'FF0000',
			'MINUTE_HAND_COLOUR': 'FF0000',
			'TIME_COLOUR': 'FFFFFF',
			'DATE_COLOUR': 'FFFFFF',
			'BG_COLOUR': '000000',
			'TICKS_COLOUR': 'FFFFFF',
			'TICKS_SIZE': '2',
			'TEMP_NOW_COLOUR': 'FFFFFF',
			'TEMP_RANGE_COLOUR': 'AAAAAA'
		},
		platinum: {
			'HOUR_HAND_COLOUR': 'FFFF55',
			'MINUTE_HAND_COLOUR': 'FFFF55',
			'TIME_COLOUR': 'AAAA00',
			'DATE_COLOUR': 'AAAA00',
			'BG_COLOUR': '000000',
			'TICKS_COLOUR': 'AAAA00',
			'TICKS_SIZE': '1',
			'TEMP_NOW_COLOUR': 'FFFF00',
			'TEMP_RANGE_COLOUR': 'AAAAAA'
		}
	};


	const screenshots = {
		classic: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAALQAAAC0CAMAAAAKE/YAAAAAQlBMVEUBAADAwMAAAACrYH5zKC8AFzGBgYE/Pz8AXpu0iIs7Tmyyk7F+jKp2SEtPiKYRS2qtVl02HTZyL0RVfXt0VHMxYJ2uTC1xAAAAAXRSTlMAQObYZgAAB/RJREFUeNrVnY16oywQhSEmoDXNtttv7/9WP/9HGPn1QON5dtMkor4ZxxEGUIGU9Ei8oYjuGuTEdBXymeNK3DPBlbjlaYnaChI9Ho/3wo6w4scA/fFG5o469I9Rb+Ml426Qlq6Ave0H49NVsGUxiVKSRSVKSBaXgEtWkMBKVpIAygwKJSVg2japHiO3kkgNZihAvb9kqJH8IxNPKX2bpJUyr0JwYxtWoVd12ytsfFphBd9d79EuMm+HW/qWBj3b2FppszTYseetcJ/W1v6TrEzWnk2NPR0dpw2nUGlmpvXwUcSNwXYeYK5H7TncKdDD4nrUKRwqzs5LqFO6FPV+/SCS8hfghVQRaomBVsTm/DWwa7oEQROz+/doELUEQW9g3oUKQ42C9mMpqKklCFoFTKmBppYo6IAlqcD5CrZEQa+GDl+lTjcLJAxab3Y8YWpZGXozY3FTSxi0Wq3oU0TlBc28QUctyy0EZo6Bpi+yD0dFaO6uuaWwzF5obsPs4wFl9kGrbdFpp5byzaDNYk9HKSQzDnr2opezWCXooLNy12/T/YOyBL8D/WyfyV5t5GMqQq/lXntmMqCf2sh8AaAXC0ZuQrbt099H5nOO+bUytB6ZQ72RR9C/aGn1MzAHeyOPmNE+HQutbvJP2/5EZNxRPRQIaNkOzDpiZ4AQjYJ+Dsx/BnNH6G2gXzPzpaDbmTkOGsN8GnpjvkXt7h2gnyPzP3WLhoYwn4N+jcyvpZoXt7906AcWup2YpS4KPQRwIPSznSRzoV2Qs3ZfAGt5r4WZysUpErpI1bSdtTsiMOjtzfT+gbP0ypwPLYPQo8Y/oDbic2E224iRCkFPWs9AHPRrYTaL4aElELpdtG4hC1pGuMfyB5Bhem7MmdCyPvRrZWa5vCrQn/cuPWvarmK5mjrQA/W9851G2zIPM/3qStAj9f1z2GtsT8CTmEm3LGiZr+4+Yau4PpfXxhzqcwlTy7PUg5eoiN6tdtNR71ZF6IF6wQ51XT2J+XnUpVsTevRrjs3t+DKZeYEz0I9B00v8AMG7B3ttlLQmM+8bT4SWx9CBqwrtVqu7B3syI2fmoxDSqA/jHEE/QtDDHu9O7Cmw/ASYhyJoaPrI6nzrASZqzj3k6ULMw88uBk3tGBN6onZjt35/nlZHQ4//5v9k5e0dZcTvDuznnvmfY7RYCeh9W2t+O78YlaK7jW1eBOdEkmNcHh5acujls1nfuR9gtyazYwRkCejhhUMP/+xK2ifD/k++ijDHQ1uRg0GP1FyyBHMQeno1oocD+phaepm1zJPd48mhzThNpRn0oYe81oQdzMwzNPUOnc6adjb0xxzzDucE5E/TEIBZFZs49ffkH77ZF2Hx7jmxyywipt10B04tIZq8s4ilGbVEMTffA/N3M0ngZwpZZ6LEqGkG6GbWLnpgZghZ1MvRO+/VzU7ZrS0eDQ6pP3craHuN2tCOuMupzd9oY9eE9swQIurPrrNX4GvkQHcCZebF2P4V8i+LvQENZXZmIn1rVIEOzxDizKep99C9Cxo4Q4gWUqRTeyePumbFQONmCNES7fq+NHT6DKGIQ6DSIp50QcNmCGn60r1GWej0GUKMOZzLC0M7qFEzhFYs/xmi0yKeCxo1Q2j9IrDBiNruVxgaNENohQo21wLIe+zeCQ2aIbR81vnQ971ml3ZCg2YIeaDDBXgD/+8MbVKfnyixmtr8qPKgeZPza3RpCxowJWU98zCWvtsaoKETJpdCSGgyNJm6t6ARM1JWCkj0+KTmz+bVczcz0qWpVChOh5cT6f594xnwgZoh5L8i0uJo6N4zsgY1Q8jb60ZL46FlEWgqFq4TUfUr2qe34WIloLUNpr3Vr+joIVYBgwe5vq+azRfExmlZB3pej7fPzJZO7BVxN9gUCr2Uc7RsjZweFYyse3SCBIx4G2MwIcW3F67lyXrQUp1Ienzta0vGUPXC0C5srWJyB18z8lT/LwWtDqF1Zv6xMZu0hspCK53rHl1jNGkrQi/IIQ8JG7oetMpMQHJDS++EMyT08DGQgNTefjhuaJLZyYiDZplJOgBhaj11w5GhmYxORhi0lbBk2N6N9hMLGZoLZukFxc8crjNRhycZmgvl03voPVQ4Z8nPQsvQXKjosUBbLZMQtS/pSIbmwtfyVqKsnGbji9EkDPRmXALKybV1pqFFLWjiSTd1HzA0CdtG3HByElcmcy+qQCvCz0kRNsGzkASAJsPpfOjOdg6vzsc8Bq3S022d7RyloVeEE9A9c44QNS5rmhZubi7mRgJuDBqXnz5hac4cFqwnQKfFSAdzj7lvbFyfS270aFIcmoTq3VKp4YaYSadvTRjfj6jiJpOxUr3NDLgJZHSPbfDI8Kb6IXMPuN1mUt94uLhd5e4YM+S+pkmjEKh43FiMxlaHuYVs/HgPXtzfSAQwC4EZWXMLHZrtV/UAZiEgY5gco6t4H0EHYRYCMlrMLn68im5AzEJAxuVpOytG/RckHLOTOi2d6ChXitlxy/3QWNNEasrHkHrokwLmnJkGjo99DPq2mbGPZFiykyqtp0p5oZmpTz9GgqjJ0oGR6inGVtMWv9HMfLpzYE4AF0UYe5Vli6hT0IGdNZeCyu8uTftV0GYWNR5R1JdgLk6NiXSVsXu8mStQdwXMXB4bFjRqYouCuuKjBq/5UMeLPj7zog8qvegjYUGRRPyKrkd81QdKX/XR3Vd9SHr9x9H/Dxw9qe559oycAAAAAElFTkSuQmCC",
		nautical: "data:image/png;base64,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",
		purple: "data:image/png;base64,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",
		rose: "data:image/png;base64,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",
		gold: "data:image/png;base64,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",
		red: "data:image/png;base64,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",
		black: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAALQAAAC0BAMAAADP4xsBAAAAHlBMVEUBAAAAAADAwMCBgYGrP0o/Pz84ERRzKC+tVl12SEvAy7fDAAAAAXRSTlMAQObYZgAAB49JREFUeNrNm2tuo0gQx50b8DD27EeDfABs+QCRkDP5FmBzgEjZA0TKzp4gB1hpDrz9Lrqru6shIG0pM8YN/PhT/e5y79Ist2y3mj3kyLLdGga8laUbxasrl5ANhCvJGwhH5Pq0EhtpLOtmHadgSlHXqzic34VVr5GZ4ibs6xUKipWDq5YTRt6IzchbsX13r1Mv83wr9oMsw1u4hF9ZnfIzzZtqSHZH2edjhFeNdV3fen54qdtU9oPUIv6GWliPyLW0G69Guh5ltOiJ6tqPLmtt54nqPEG08XVp0Fiz0Q35nZFkyJ3Bj77IVHwuSy/ScDt2B7hkYpRo/OK9+8Alsu0Sa+7Gos8sbcSyZzVLpYu+AK+aITun0ZUkw0m7ewiLptHFRKgsQ01KU5InoAdLZ4Wy4htow4IntTT6IYoGmWfn9Jn2SJ6ALlyVrrPz9A6xxA5gH04CJTtPQV+kyMhr5EvRunyA81GK1x80utIarYedCY/kKejS28Q6vSgSnYx2Lhi090H2InRh0FZSHJ2noY3E2Is4otPQg3FszP3ZSuhKXnAIovNlaCiPH9O0JeiLqR9gAr23klx/sJsWow/3oLP5u+ZFPxMNaZ9v9nTHRt/yqo2h4eUxen93Jmm2P07sbyG67d6cqWUG6G+pPv++u1NLQOfK14vQw4/uXzS1NGiduAjdd8/4dd2quAz93n1hUdka6GP3c9gI3XVfdRidfwO9Z6J9ufR99KHr/q7D6IdvoD+7p8GLzoLoUxwN7ug+LvPQRR1Hgzu6PIb29aGnaMsH7ugeZRo2Hzq9Ud0z0fpNaDS/sS5ORZJqRn6Moh8QuuZoum98F6KrADpz0VwV+0tBH7lo1aMno/MkdMeMXxBB58gh/JMcPb1z0XABtsXoIxdtCtV89PEeHKly8qMZqS5Q/Xm3c8l823O0GV/PQ+vXfi59s4KDEs2vDqGD7Z7JrOfKM5f55Gh7LoOLSAyt5P2FZmB7IRpmYHPRUMbe7Gkifx5PNQn9MvQfhgIihTvuaLaL0fqSwuuz6k/FhnzcS9EwRw+h8wlaFg2rBpQ3BTLXtSLhjlYWcBEx9zAsf2EX3WiRinX+rb7DekgiWjZPpgFkb3zqpjKLH/obrOLMQMt2oVDoem+xO8fT7IZE9KlmR0Jzwf9jd3PZwH4Xh0/TFbN0NIfzg4J9SmF7YB/l0dd0nS8ZLXXzLwbdd4YtP38OzuokjZZEg2b/JPqo2U/y6Gu6pjoPrcpHrdGsOdK2d0XPQReqhEzRR0Dbots8huaMKVqVa5Gs0SD7H+aV6ap7DB2LZgD6oNG/WGtqxQrCcbidiWZQjau0x66DCEdYTyPRBf+j7GDQjymxmnqimpat0bGLoC+6Xq/a17Spkv2RcOm1bjg6HO9yfLqXaOtECC0sNkdySsIEPeoTS9GT8guyVTOt2bPROGokZf96U2SojkH0zUJTUSN4mWgjUgXQdNTogk/Q6KSoUememIvGUSNLdPPKjl7FiVMSmogawZfGOabRRNQIx3xKr0dKgW4tdDxqBG/T4ggQjaajRuYhMOYLoO2RKh01Kh2ZF4+zR4F+sdF01KjwxMTQrQ1G01EjQOHJ37TUIDQRNVKfTRTNvku2PSugokaARnMyt65y9LSIUFEjNUuPoQeNvtnzRjJqRKuulTUOmooa0egqhKaiRvESAsVe9BE2mooa4XIN3+GrynN7qYWOGuHaKJMx+sVB01GjatqGwFeMzkg0Wq+42Gsv6iWwr8UaYjq6VbAGtWK4hDjrfGRoR9/cOu04LtenuWhxDpSOuvfBtbF1lmvJqBG0Ejcx7uPmH2K8OGgqanTSwiLDJ+XqbBG6io5wStmmqmDBDLTLbnrce3HybTcHPQD6AujAKLXVwZNktEv2FH+JfpmPxsPJm2+gcM0WocfocHKUg7IMh9dodFVPh5O4NjbS1dpgGkmjB+Fhk5+41+XoHkKZstmh0TDMnLAb+yLhakBr1XS5Hiz3QmMFF0lXG9O+ptG1Z3BwhifJyWi7A0utjaZXsWVPy4dyNXgkseWzexW8Xns1pRostb0evD/va+2qeNstQIuxYHi4PJoGxPUI3TdKDh5W2f54WYDuBT/4Y5zyqmu5ZWnjkDKKBn+4sunRUy/+Cw3iKuMPGo0JUfRo/OFYykjVpGM0iG6J35H6x9dR1aWpL8ioWYFBB7LxqusLtpS5TLCEgOje91NBegYGb4OrzBX8gY2cN/aeYJpOGiETfbKp2S68DRpRV1dTqL1GztGhCXUXBkaTiX6jVxZke427gvIKmej3CLEeog9c0Y3KQ6iJ2BJWcaBDh2eeRiQay6bXngbT78Ig7RWLxkavmJX6GB7UXCOiQXZknQ8eqdeXX+Vxg0TTsi3rTSa4BqJTf/wfWlMNom/UxgL4vVtgJXj6SMshL4mbZViJqALzrMGPbuntIUa1veoeyIX+otEJO2bgN0TBqNEIEzB2ocnDRHY8kFG9ind5hYFNu8l+qlG4Y5NtTyV3xzZbqirmjq22mI23zbavVdttusv+L5sQt9w6ueWGzy23qW65uXbLLcFbbmTedPv1lpvGN93qvniD/n/n49FxTiSDWAAAAABJRU5ErkJggg==",
		platinum: "data:image/png;base64,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"
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
