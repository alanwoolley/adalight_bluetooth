#define FASTLED_ALLOW_INTERRUPTS 1
#define FORCE_SOFTWARE_SPI
#include <SoftwareSerial.h>
#include <FastLED.h>
#include <elapsedMillis.h>


// Define the number of LEDs
#define NUM_LEDS 46

// Define SPI Pin
#define PIN 6

// Serial rate of Bluetooth interface
#define serialRate 57600

// Milliseconds of inactivity before disabling LEDs
#define idleTimeout 10000

// Adalight "Magic Word"
uint8_t prefix[] = {'A', 'd', 'a'}, hi, lo, chk, i;

// initialise LED-array
CRGB leds[NUM_LEDS];

SoftwareSerial BTserial(2, 3); // RX | TX
// Connect the HC-05 TX to Arduino pin 2 RX. 
// Connect the HC-05 RX to Arduino pin 3 TX through a voltage divider.

void setup()
{
  BTserial.begin(serialRate);
  
  FastLED.addLeds<WS2812, PIN, RGB>(leds, NUM_LEDS);
  
  // initial RGB flash
  LEDS.showColor(CRGB(255, 0, 0));
  delay(500);
  LEDS.showColor(CRGB(0, 255, 0));
  delay(500);
  LEDS.showColor(CRGB(0, 0, 255));
  delay(500);
  LEDS.showColor(CRGB(0, 0, 0));
  
  BTserial.print("Ada\n");
}

void loop() { 
  elapsedMillis waiting;
  boolean ledsEnabled = true;
  
  // wait for first byte of Magic Word
  for(i = 0; i < sizeof prefix; ++i) {
    waitLoop: while (!BTserial.available()) {
      if (ledsEnabled && (waiting > idleTimeout)) {
        LEDS.showColor(CRGB(0, 0, 0));
        ledsEnabled = true;
      }
    }
    // Check next byte in Magic Word
    if(prefix[i] == BTserial.read()) continue;
    // otherwise, start over
    i = 0;
    goto waitLoop;
  }
  
  // Hi, Lo, Checksum
  
  while (!BTserial.available()) ;;
  hi=BTserial.read();
  while (!BTserial.available()) ;;
  lo=BTserial.read();
  while (!BTserial.available()) ;;
  chk=BTserial.read();
  
  // if checksum does not match go back to wait
  if (chk != (hi ^ lo ^ 0x55))  {
    goto waitLoop;
  }
  
  memset(leds, 0, NUM_LEDS * sizeof(struct CRGB));
  BTserial.readBytes((char*)leds, NUM_LEDS*3);
  // shows new values
  FastLED.show();

  // Flush buffer
  while(BTserial.available() > 0) { BTserial.read(); } 
}
