/* 

Zach Hensel
23 March 2019

This is a modification of the Arduino sketch provided with the Micro-mangager Arduino device adapter; 
it outputs 5 TTL signals to pins 8, 9, 10, 11, and 12, but not to pin 13. Instead, commands from
the micro-manager device adapter that would trigger pin 13 turn on a Neopixel LED strip/ring. Also,
the brightness and color of the Neopixel LEDs can be modified by installing the Arduino DAC peripherals.

Installing in micromanager:

1. Install Arduino device adapter with peripherals Shutter, Switch, DAC1, DAC2
2. DAC1 = LED brightness setting; the "max volts" setting can be anything.
3. DAC2 = LED color; the "max volts" setting MUST stay at the default of 5 for this code to work, and 
    color settings 0, 1, 2, 3 are defined below (White, Red, Green, Blue); the code could be modified
    to have up to 4095 different colors.

Usage: Identical to what's described for the Arduino device adapter: https://micro-manager.org/wiki/Arduino
    with the exception of the two DAC channels

We use this to trigger 5 oxxius lasers (fluorescence) and a Neopixel 12 ring (brightfield; maybe could
    be used for fluorescence.

See our lab's implementation of this here - https://github.com/zach-hensel/lasers-and-brightfield

*/

/*
 * This goal of the application is to set the digital output on pins 8-12 
 * This can be accomplished in three ways.  First, a serial command can directly set
 * the digital output pattern.  Second, a series of patterns can be stored in the 
 * Arduino and TTLs coming in on pin 2 will then trigger to the consecutive pattern (trigger mode).
 * Third, intervals between consecutive patterns can be specified and paterns will be 
 * generated at these specified time points (timed trigger mode).
 *
 * Interface specifications:
 * digital pattern specification: single byte, bit 0 corresponds to pin 8, 
 *   bit 1 to pin 9, etc..  Bits 7 and 8 will not be used (and should stay 0).
 *   ZH190317 -- modified code so that bit 6 sets a neopixel ring rather than laser trigger
 *
 * Set digital output command: 1p
 *   Where p is the desired digital pattern.  Controller will return 1 to 
 *   indicate succesfull execution.
 *
 * Get digital output command: 2
 *   Controller will return 2p.  Where p is the current digital output pattern
 *
 * Set Analogue output command: 3xvv
 *   Where x is the output channel (either 1 or 2), and vv is the output in a 
 *   12-bit significant number.
 *   Controller will return 3xvv:
 *   ZH190317 -- hijacked this command to instead change LED intensity (channel 1) and color (channel 2)
 *
 * Get Analogue output:  4
 *
 *
 * Set digital patten for triggered mode: 5xd 
 *   Where x is the number of the pattern (currently, 12 patterns can be stored).
 *   and d is the digital pattern to be stored at that position.  Note that x should
 *   be the real number (i.e., not  ASCI encoded)
 *   Controller will return 5xd 
 *
 * Set the Number of digital patterns to be used: 6x
 *   Where x indicates how many digital patterns will be used (currently, up to 12
 *   patterns maximum).  In triggered mode, after reaching this many triggers, 
 *   the controller will re-start the sequence with the first pattern.
 *   Controller will return 6x
 *
 * Skip trigger: 7x
 *   Where x indicates how many digital change events on the trigger input pin
 *   will be ignored.
 *   Controller will respond with 7x
 *
 * Start trigger mode: 8
 *   Controller will return 8 to indicate start of triggered mode
 *   Stop triggered a 9. Trigger mode will  supersede (but not stop) 
 *   blanking mode (if it was active)
 * 
 * Stop Trigger mode: 9
 *   Controller will return 9x where x is the number of triggers received during the last
 *   trigger mode run
 *
 * Set time interval for timed trigger mode: 10xtt
 *   Where x is the number of the interval (currently, 12 intervals can be stored)
 *   and tt is the interval (in ms) in Arduino unsigned int format.  
 *   Controller will return 10x
 *
 * Sets how often the timed pattern will be repeated: 11x
 *   This value will be used in timed-trigger mode and sets how often the output
 *   pattern will be repeated. 
 *   Controller will return 11x
 *  
 * Starts timed trigger mode: 12
 *   In timed trigger mode, digital patterns as set with function 5 will appear on the 
 *   output pins with intervals (in ms) as set with function 10.  After the number of 
 *   patterns set with function 6, the pattern will be repeated for the number of times
 *   set with function 11.  Any input character (which will be processed) will stop 
 *   the pattern generation.
 *   Controller will retun 12.
 * 
 * Start blanking Mode: 20
 *   In blanking mode, zeroes will be written on the output pins when the trigger pin
 *   is low, when the trigger pin is high, the pattern set with command #1 will be 
 *   applied to the output pins. 
 *   Controller will return 20
 *
 * Stop blanking Mode: 21
 *   Stops blanking mode.  Controller returns 21
 *
 * Blanking mode trigger direction: 22x
 *   Sets whether to blank on trigger high or trigger low.  x=0: blank on trigger high,
 *   x=1: blank on trigger low.  x=0 is the default
 *   Controller returns 22
 * 
 * Get Identification: 30
 *   Returns (asci!) MM-Ard\r\n
 *
 * Get Version: 31
 *   Returns: version number (as ASCI string) \r\n
 *
 * Read digital state of analogue input pins 0-5: 40
 *   Returns raw value of PINC (two high bits are not used)
 *
 * Read analogue state of pint pins 0-5: 41x
 *   x=0-5.  Returns analogue value as a 10-bit number (0-1023)
 *
 *
 * 
 * Possible extensions:
 *   Set and Get Mode (low, change, rising, falling) for trigger mode
 *   Get digital patterm
 *   Get Number of digital patterns
 */

#include <Adafruit_NeoPixel.h>
#define PIN 6 // neopixel drive pin
#define NP 12 // number of neopixels to use
#define BF 32 // initial brightfield level for neopixels (0-255)
#define LS 1 // whether or not lightshow should play when Arduino initialized (when it is pinged for its version)

// Initialize Neopixel ring

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NP, PIN, NEO_GRB + NEO_KHZ800); // have to change this line for different types of LEDs

unsigned int LEDintensity_ = BF; // set initial LED intensity
uint32_t LEDcolor_ = strip.Color(LEDintensity_,LEDintensity_,LEDintensity_);

uint32_t black_ = strip.Color(0,0,0); // define black for turning off LEDs

bool NPon_ = false; // variable to track if neopixel array is on or off

// Unmodified code from mm arduino sketch starting here; only modified to call "modPORTB" rather than setting PORTB directly
 
unsigned int version_ = 2;

// pin on which to receive the trigger (2 and 3 can be used with interrupts, although this code does not use interrupts)
int inPin_ = 2;
// to read out the state of inPin_ faster, use 
int inPinBit_ = 1 << inPin_;  // bit mask 

// pin connected to DIN of TLV5618
int dataPin = 3;
// pin connected to SCLK of TLV5618
int clockPin = 4;
// pin connected to CS of TLV5618
int latchPin = 5;

const int SEQUENCELENGTH = 12;  // this should be good enough for everybody;)
byte triggerPattern_[SEQUENCELENGTH] = {0,0,0,0,0,0,0,0,0,0,0,0};
unsigned int triggerDelay_[SEQUENCELENGTH] = {0,0,0,0,0,0,0,0,0,0,0,0};
int patternLength_ = 0;
byte repeatPattern_ = 0;
volatile long triggerNr_; // total # of triggers in this run (0-based)
volatile long sequenceNr_; // # of trigger in sequence (0-based)
int skipTriggers_ = 0;  // # of triggers to skip before starting to generate patterns
byte currentPattern_ = 0;
const unsigned long timeOut_ = 1000; // ZH made timeout longer for testing with serial monitor
bool blanking_ = false;
bool blankOnHigh_ = false;
bool triggerMode_ = false;
boolean triggerState_ = false;
 
void setup() {
   // Higher speeds do not appear to be reliable
   Serial.begin(57600);
  
   pinMode(inPin_, INPUT);
   pinMode (dataPin, OUTPUT);
   pinMode (clockPin, OUTPUT);
   pinMode (latchPin, OUTPUT);
   pinMode(8, OUTPUT);
   pinMode(9, OUTPUT);
   pinMode(10, OUTPUT);
   pinMode(11, OUTPUT);
   pinMode(12, OUTPUT);
   pinMode(13, OUTPUT);
   
   // Set analogue pins as input:
   DDRC = DDRC & B11000000;
   // Turn on build-in pull-up resistors
   PORTC = PORTC | B00111111;
   
   digitalWrite(latchPin, HIGH);

   strip.begin();
   strip.show();
    
 }
 
 void loop() {
   if (Serial.available() > 0) {
     int inByte = Serial.read();
     switch (inByte) {
       
       // Set digital output
       case 1 :
          if (waitForSerial(timeOut_)) {
            currentPattern_ = Serial.read();
            // Do not set bits 6 and 7 (not sure if this is needed..)
            currentPattern_ = currentPattern_ & B00111111;
            
            if (!blanking_)
              PORTB = modPORTB(currentPattern_);
              
            Serial.write( byte(1));
          }
          break;
          
       // Get digital output
       case 2:
          Serial.write( byte(2));
          Serial.write( PORTB);
          break;
          
       // Set Analogue output (TODO: save for 'Get Analogue output')
       case 3:
         if (waitForSerial(timeOut_)) {
           int channel = Serial.read();
           if (waitForSerial(timeOut_)) {
              byte msb = Serial.read();
              if (waitForSerial(timeOut_)) {
                byte lsb = Serial.read();
                analogueOut(channel, msb, lsb);
                //LEDintensity_ = 255;
                Serial.write( byte(3));
                Serial.write( channel);
                Serial.write(msb);
                Serial.write(lsb);
              }
           }
         }
         break;
         
       // Sets the specified digital pattern
       case 5:
          if (waitForSerial(timeOut_)) {
            int patternNumber = Serial.read();
            if ( (patternNumber >= 0) && (patternNumber < SEQUENCELENGTH) ) {
              if (waitForSerial(timeOut_)) {
                triggerPattern_[patternNumber] = Serial.read();
                triggerPattern_[patternNumber] = triggerPattern_[patternNumber] & B00111111;
                Serial.write( byte(5));
                Serial.write( patternNumber);
                Serial.write( triggerPattern_[patternNumber]);
                break;
              }
            }
          }
          Serial.write( "n:");//Serial.print("n:");
          break;
          
       // Sets the number of digital patterns that will be used
       case 6:
         if (waitForSerial(timeOut_)) {
           int pL = Serial.read();
           if ( (pL >= 0) && (pL <= 12) ) {
             patternLength_ = pL;
             Serial.write( byte(6));
             Serial.write( patternLength_);
           }
         }
         break;
         
       // Skip triggers
       case 7:
         if (waitForSerial(timeOut_)) {
           skipTriggers_ = Serial.read();
           Serial.write( byte(7));
           Serial.write( skipTriggers_);
         }
         break;
         
       //  starts trigger mode
       case 8: 
         if (patternLength_ > 0) {
           sequenceNr_ = 0;
           triggerNr_ = -skipTriggers_;
           triggerState_ = digitalRead(inPin_) == HIGH;
           PORTB = modPORTB(B00000000);
           Serial.write( byte(8));
           triggerMode_ = true;           
         }
         break;
         
         // return result from last triggermode
       case 9:
          triggerMode_ = false;
          PORTB = modPORTB(B00000000);
          Serial.write( byte(9));
          Serial.write( triggerNr_);
          break;
          
       // Sets time interval for timed trigger mode
       // Tricky part is that we are getting an unsigned int as two bytes
       case 10:
          if (waitForSerial(timeOut_)) {
            int patternNumber = Serial.read();
            if ( (patternNumber >= 0) && (patternNumber < SEQUENCELENGTH) ) {
              if (waitForSerial(timeOut_)) {
                unsigned int highByte = 0;
                unsigned int lowByte = 0;
                highByte = Serial.read();
                if (waitForSerial(timeOut_))
                  lowByte = Serial.read();
                highByte = highByte << 8;
                triggerDelay_[patternNumber] = highByte | lowByte;
                Serial.write( byte(10));
                Serial.write(patternNumber);
                break;
              }
            }
          }
          break;

       // Sets the number of times the patterns is repeated in timed trigger mode
       case 11:
         if (waitForSerial(timeOut_)) {
           repeatPattern_ = Serial.read();
           Serial.write( byte(11));
           Serial.write( repeatPattern_);
         }
         break;

       //  starts timed trigger mode
       case 12: 
         if (patternLength_ > 0) {
           PORTB = modPORTB(B00000000);
           Serial.write( byte(12));
           for (byte i = 0; i < repeatPattern_ && (Serial.available() == 0); i++) {
             for (int j = 0; j < patternLength_ && (Serial.available() == 0); j++) {
               //PORTB = triggerPattern_[j];
               PORTB = modPORTB(triggerPattern_[j]); //ZH
               delay(triggerDelay_[j]);
             }
           }
           PORTB = modPORTB(B00000000);
         }
         break;

       // Blanks output based on TTL input
       case 20:
         blanking_ = true;
         Serial.write( byte(20));
         break;
         
       // Stops blanking mode
       case 21:
         blanking_ = false;
         Serial.write( byte(21));
         break;
         
       // Sets 'polarity' of input TTL for blanking mode
       case 22: 
         if (waitForSerial(timeOut_)) {
           int mode = Serial.read();
           if (mode==0)
             blankOnHigh_= true;
           else
             blankOnHigh_= false;
         }
         Serial.write( byte(22));
         break;
         
       // Gives identification of the device
       case 30:
         Serial.println("MM-Ard");
         break;
         
       // Returns version string
       case 31:
         Serial.println(version_);
         
         if (LS) {
              // Light show after everything started up OK
         for(int i=0;i<NP;i++){
          strip.setPixelColor(i, BF, 0, 0);
          delay(25);
          strip.show();
        }
        for(int i=0;i<NP;i++){
          strip.setPixelColor(i, 0, BF, 0);
          delay(25);
          strip.show();
        }
        for(int i=0;i<NP;i++){
          strip.setPixelColor(i, 0, 0, BF);
          delay(25);
          strip.show();
        }
        for(int i=0;i<NP;i++){
          strip.setPixelColor(i, 0, 0, 0);
          delay(25);
          strip.show();
        }
         }
       
         break;

       case 40:
         Serial.write( byte(40));
         Serial.write( PINC);
         break;
         
       case 41:
         if (waitForSerial(timeOut_)) {
           int pin = Serial.read();  
           if (pin >= 0 && pin <=5) {
              int val = analogRead(pin);
              Serial.write( byte(41));
              Serial.write( pin);
              Serial.write( highByte(val));
              Serial.write( lowByte(val));
           }
         }
         break;
         
       case 42:
         if (waitForSerial(timeOut_)) {
           int pin = Serial.read();
           if (waitForSerial(timeOut_)) {
             int state = Serial.read();
             Serial.write( byte(42));
             Serial.write( pin);
             if (state == 0) {
                digitalWrite(14+pin, LOW);
                Serial.write( byte(0));
             }
             if (state == 1) {
                digitalWrite(14+pin, HIGH);
                Serial.write( byte(1));
             }
           }
         }
         break;

       }
    }
    
    // In trigger mode, we will blank even if blanking is not on..
    if (triggerMode_) {
      boolean tmp = PIND & inPinBit_;
      if (tmp != triggerState_) {
        if (blankOnHigh_ && tmp ) {
          PORTB = modPORTB(0);
        }
        else if (!blankOnHigh_ && !tmp ) {
          PORTB = modPORTB(0);
        }
        else { 
          if (triggerNr_ >=0) {
            //PORTB = triggerPattern_[sequenceNr_];
            PORTB = modPORTB(triggerPattern_[sequenceNr_]); //ZH
            sequenceNr_++;
            if (sequenceNr_ >= patternLength_)
              sequenceNr_ = 0;
          }
          triggerNr_++;
        }
        
        triggerState_ = tmp;       
      }  
    } else if (blanking_) {
      if (blankOnHigh_) {
        if (! (PIND & inPinBit_))
          PORTB = modPORTB(currentPattern_);
        else
          PORTB = modPORTB(0);
      }  else {
        if (! (PIND & inPinBit_))
          PORTB = modPORTB(0);
        else  
          PORTB = modPORTB(currentPattern_);
      }
    }
}
 
bool waitForSerial(unsigned long timeOut)
{
    unsigned long startTime = millis();
    while (Serial.available() == 0 && (millis() - startTime < timeOut) ) {}
    if (Serial.available() > 0)
       return true;
    return false;
 }

// New functions added by ZH below this point

// Writes to PORTB and to neopixel LEDs dending on pin 13 state
byte modPORTB(byte tempPattern) {
  
  // Check for bit 5 for LED command
  if ((tempPattern >> 5) & 0x1) {
    // tempPattern = tempPattern & B00011111; // this is not necessary I think; removing it will make pin 13-linked LED light up at same time as neopixels which is useful for diagnosing problems
    // Turn on bf LEDs if off
    if (! NPon_) {
      strip.fill(LEDcolor_);
      strip.show();
      NPon_ = true;
    }
  }
  else if (NPon_) {
    // Turn off bf LEDs
    strip.fill(black_);
    strip.show();
    NPon_ = false;
  }
  return tempPattern;
}

void analogueOut(int channel, byte msb, byte lsb) 
{

  if (channel == 0) {
    float channelData = (lsb & 0xFF) | ((msb & 0x0F) << 8);
    LEDintensity_ = 255*channelData/4095;
    LEDcolor_ = strip.Color(LEDintensity_,LEDintensity_,LEDintensity_); // set to white; could change to maintain color but it's annoying to code
  }
  else {
    float channelData = (lsb & 0xFF) | ((msb & 0x0F) << 8);
    int color = 5*channelData/4095; // can change 5 to something else for more colors but this will work for now
  
    switch (color) {
       
       // Set LED color options
       case 0 : // White
          LEDcolor_ = strip.Color(LEDintensity_,LEDintensity_,LEDintensity_);
          break;
       case 1 : // Red
          LEDcolor_ = strip.Color(LEDintensity_,0,0);
          break;
        case 2 : // Green
          LEDcolor_ = strip.Color(0,LEDintensity_,0);
          break;
        case 3 : // Blue
          LEDcolor_ = strip.Color(0,0,LEDintensity_);
          break;
    }
    
  }

  // If Neopixel on, update new intensity
  if (NPon_) {
    NPon_ = false;
    PORTB = modPORTB(currentPattern_); 
  }
  
}
