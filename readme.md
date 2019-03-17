# Description

This describes a small hack of the [Micromanager Arduino Sketch](https://valelab4.ucsf.edu/svn/micromanager2/trunk/DeviceAdapters/Arduino/), which can trigger 5 lasers (Oxxius LaserBoxx LBX and LBC models) and 1 neopixel LED ring (rather than 6 lasers). The motivation for doing this was a low-effort way to add brightfield illumination to a microscope setup that already had 5  lasers triggered using the Arduino device adapter in micromanager without learning how to modify device adapters.

This was inspired by seeing a similar setup in the [Oxford Nanoimager](https://www.youtube.com/watch?v=QzGPyz0SOf8), which looks to have 6 LEDs in a circle with a circumference similar to the Neopixel 12 ring placed a few cm above the sample. With a phase objective, it is possible to achieve [condenser free phase contrast](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4277858/) using an LED ring, as well as other contrast methods.

All 3d printed components were fabricated by [MiniLab3d](https://minilab3d.pt) via 3dhubs and STL files are in 1-mm units.

## Software

* The Arduino sketch was modified so that commands to trigger the 6th laser instead turn on each pixel in an Adafruit neopixel LED ring. The LED brightness is hardcoded into the Arduino sketch in a #define command; a future update will use the "Set Analogue output" command to instead set the LED intensity since this implementation is not using the analogue output (DAC) device.

### Installation

1. Modify parameters in #define statements in the Arduino sketch including:
	* #define BF 20 // default LED intensity (8-bit 0—255)
		* TO DO: Make global BF variable that can be changed by receiving command 31vv where vv is 12-bit number from 0 to 4095
	* #define PN 12 // number of pixels in the LED ring
	* #define LS 1 // runs a lightshow whenever pinged for the controller version; set 0 to turn off
		* TO DO: move the light show code to controller version section of sketch and run only after returning the version to serial so that it doesn't interfere with initialization routine

## Control box hardware

* Arduino Uno Rev3 powered by USB (From Elegoo starter kit)
* Arduino Proto Shield V5 with some headers removed (from Elegoo starter kit; plastic headers removed with pliers and then pins removed by desoldering)
* 5x female db9 connectors
	* Arduino outputs 8--12 wired to pin 5 (Laser Enable)
	* Arduino ground wired to pin 9 (Digital Ground)
	* Connected to Oxxius lasers using straight-through male-female db9 cable
* 1x female XLR connector (sourced from PT Robotics #PTR002086)
	* Connected to Neopixel LED ring using male-female XLR cable
	* Pin 1 to Arduino ground
	* Pin 2 to Arduino digital output 6
	* Pin 3 to Arduino +5 V
* 1x female BNC connector (sourced from PT Robotics #PTR002965)
	* Connected to triggering device (e.g. camera trigger out); +5 input tested to work with +3.3 V trigger outputs from EMCCD (Photometrics Evolve 512) and sCMOS (Photometrics Prime 95b), but may want to [convert to +5 V](https://github.com/PRNicovich/NicoLase/tree/master/Hardware).
	* Shield to Arduino ground
	* Conductor to Arduino digital input pin 2
* ABS Enclosure cut with rotary tool to fit back panel (sourced from PT Robotics #PTR000810)
* Back panel printed 0.1-mm layers in PLA.
	
## LED ring hardware

* 3d printed neopixel ring holder. Both were printed 0.1-mm layer PLA; in retrospect the LED holder would've been better to print ABS because it warps after fitting to the ball mount. STL files sourced from:
	* Base (can be attached to optical breadboard with M6 screws)
		* MK3_Camera_Ball_Mount.stl from user briankb at thingiverse
		* https://www.thingiverse.com/thing:2828182
	* Light ring
		* picamneopixel holder.stl from user nixternal at thingiverse
		* https://www.thingiverse.com/thing:2189609
		* Modified with rotary tool to add a hole for XLR connector, which was superglued in place
		
* Male XLR connector (sourced from PT Robotics #PTR002090) 
	* Drill hole to accomodate XLR connector within LED ring
	* Super glue in place

* Adafruit Neopixel Ring w/ 12x WS2812 RGB LEDs (sourced PT Robotics #PTR003298)
	* XLR connector Pin 1 to GND
	* XLR connector Pin 2 to IN
	* XLR connector Pin 3 to PWR