# Gauge Geometry
An analog-digital Pebble watch face designed for legibility and simplicity

<img src="screenshots/screenshot-1-classic.png?raw=true" width="90" height="90"/> <img src="screenshots/screenshot-2-nautical.png?raw=true" width="90" height="90"/> <img src="screenshots/screenshot-3-gold.png?raw=true" width="90" height="90"/> <img src="screenshots/screenshot-4-rose.png?raw=true" width="90" height="90"/>

Large digits and analog hands make it easy to read the time at a glance from more angles. The watch face also displays day/date, temperature, and battery level. Colours and hand shapes are customisable from the settings page.

## Installation
Get it from the [Rebble appstore](https://apps.rebble.io/en_US/application/5fd419293dd3100155d398b8). Or download the latest `.pbw` binary from the [releases page](https://github.com/szupie/gauge-geometry/releases) on GitHub and sideload it to your device.

## Building
To build this project, you’ll need the Pebble SDK installed ([setup guide](https://github.com/andb3/pebble-setup)). Then run `pebble build`, and 
`pebble install --emulator [basalt, chalk]` or `pebble install --phone phone_ip` to run on the emulator or a Pebble.

## Credits
This project uses [Clay](https://github.com/pebble/clay), [GBitmap Colour Palette Manipulator](https://github.com/rebootsramblings/GBitmap-Colour-Palette-Manipulator), and [pebble-events](https://github.com/Katharine/pebble-events).

Digital time is set in Century Gothic. Day/date text is set in [Poppins](https://github.com/itfoundry/poppins).
