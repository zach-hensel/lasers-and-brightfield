# Description

This describes a small hack of the [Micromanager Arduino Device Adapter](https://valelab4.ucsf.edu/svn/micromanager2/trunk/DeviceAdapters/Arduino/), which can trigger 5 lasers and 1 neopixel LED ring.

## Software

## Control box hardware

* Arduino Uno powered by USB (Elegoo used in build)
* Arduino Proto Shield V5 with headers removed (Elegoo used in build)
* 5x female db9 connectors
	* Arduino outputs 8--12 wired to pin X
	* Arduino ground wired to pin Y
	* Connected to Oxxius lasers using straight-through male-female db9 cable
* 1x female XLR connector
	* Connected to Neopixel LED ring using male-female XLR cable
* 1x female BNC connector
	* Connected to triggering device (e.g. camera trigger out); +5 input tested to work with +3.3 V outputs from EMCCD (Photometrics Evolve 512) and sCMOS (Photometrics Prime 95b), but may want to [convert to +5 V](https://github.com/PRNicovich/NicoLase/tree/master/Hardware)
	
## LED ring hardware

* Adafruit Neopixel 12 Part Z
* 3d printed connector
	* Base
	* Light right
* Male XLR connector 
	* Drill hole to accomodate XLR connector within LED ring
	* *