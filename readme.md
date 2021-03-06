# Description

<img src="images/sequentialTriggering.mov.gif" width="284">

This describes a small hack of the [Micromanager Arduino Sketch](https://valelab4.ucsf.edu/svn/micromanager2/trunk/DeviceAdapters/Arduino/), which can trigger 5 lasers (Oxxius LaserBoxx LBX and LBC models) and 1 neopixel LED ring (rather than 6 lasers). The motivation for doing this was a low-effort way to add brightfield illumination to a microscope setup that already had 5  lasers triggered using the Arduino device adapter in micromanager without learning how to modify device adapters.

This has been tested with micromanager 2.0beta and 2.0gamma with the default arduino device adapter.

<img src="images/digitalOuts.jpeg" width="400">

This was inspired by seeing a similar setup in the [Oxford Nanoimager](https://www.youtube.com/watch?v=QzGPyz0SOf8), which looks to have 6 LEDs in a circle with a circumference similar to the Neopixel 12 ring placed a few cm above the sample. With a phase objective, it is possible to achieve [condenser free phase contrast](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4277858/) using an LED ring, as well as other contrast methods.

All 3d printed components were fabricated by [MiniLab3d](https://minilab3d.pt) via 3dhubs and STL files are in 1-mm units.

## Software

* The Arduino sketch was modified so that commands to trigger the 6th laser instead turn on each pixel in an Adafruit neopixel LED ring. The LED brightness is hardcoded into the Arduino sketch in a #define command; a future update will use the "Set Analogue output" command to instead set the LED intensity since this implementation is not using the analogue output (DAC) device.

* "AOTFcontroller_NP_color.ino" adds the ability to change the color of the Neopixel LEDs and uses some more compact code for assigning LED colors. This is the version that is still under development. DAC channel 2 is used to change LED color (right now, by setting Max Volts to 5 and setting output voltage to 0, 1, 2 or 3 for white, red, green or blue). Up to 4095 colors could be added easily; 24-bit color is technically possible but would be confusing to accomplish with the current Arduino device adapter.

### Installation

1. Modify parameters in #define statements in the Arduino sketch including:
	* `#define BF 20 // default LED intensity (8-bit 0—255)`
	* `#define PN 12 // number of pixels in the LED ring`
	* `#define LS 1 // runs a lightshow whenever pinged for the controller version; set 0 to turn off`
	* `#define PIN 6 // digital out pint to communicate with neopixel ring`
		
2. Wire an Arduino uno to (see below for our specific implementation):
	* Output to trigger TTL devices on pins 8, 9, 10, 11, 12
	* Input on pin 2
	* Output data to Neopixel ring on pin 6
	
3. Compile and upload sketch to Arduino.

4. Install Arduino device adapter including Switch and DAC components; the Neopixel ring will give a short light show if it is configured correctly.

5. Configure settings according to the instructions for the [Arduino device adapter](https://micro-manager.org/wiki/Arduino) with these modifications:
	* A digital output pattern with Pin 13 on will turn on the Neopixel ring (rather than triggering a 6th TTL device)
	* DAC-Volts can be adjusted from 0 (minimum neopixel LED intensity) to 5 (maximum  intensity)
	* Note: If the Neopixel ring is not turning on, the line "tempPattern = tempPattern & B00011111;" can be commented out in the Arduino sketch to diagnose if this is a problem with configuration or connecting the Neopixel ring, because the Arduino Uno has an LED indicating Pin 13 digital out.

## Control box hardware

<img src="images/internal.jpeg" width="400">
<img src="images/external.png" width="250">
<img src="images/assembledControlBox.jpeg" width="250">

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
	* In our implementation a push-button can also trigger Pin 2 from the +5 Arduino and a 10 kOhm pull-down resistor is also added
* ABS Enclosure cut with rotary tool to fit back panel (sourced from PT Robotics #PTR000810)
* Back panel printed 0.1-mm layers in PLA.
	
## LED ring hardware

<img src="images/ledHolder.jpeg" width="250">

* 3d printed neopixel ring holder. Both were printed 0.1-mm layer PLA; in retrospect the LED holder would've been better to print ABS because it warps after fitting to the ball mount. STL files sourced from:
	* Base (can be attached to optical breadboard with M6 screws)
		* MK3_Camera_Ball_Mount.stl from user briankb at thingiverse
		* https://www.thingiverse.com/thing:2828182
		* Licensed CC-BY
	* Light ring
		* picamneopixel holder.stl from user nixternal at thingiverse
		* https://www.thingiverse.com/thing:2189609
		* Modified with rotary tool to add a hole for XLR connector, which was superglued in place
		* Licensed CC-BY-NC-SA
		
* Male XLR connector (sourced from PT Robotics #PTR002090) 
	* Drill hole to accomodate XLR connector within LED ring
	* Super glue in place

* Adafruit Neopixel Ring w/ 12x WS2812 RGB LEDs (sourced PT Robotics #PTR003298)
	* XLR connector Pin 1 to GND
	* XLR connector Pin 2 to IN
	* XLR connector Pin 3 to PWR