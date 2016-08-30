// This #include statement was automatically added by the Particle IDE.
#include "adafruit-ina219/adafruit-ina219.h"

// This #include statement was automatically added by the Particle IDE.
#include "neopixel/neopixel.h"

// This #include statement was automatically added by the Particle IDE.
#include "OneWire/OneWire.h"

// Configurations:
// Kitchen Sink:
// Channel 1: Under Cabinet Lights
// Channel 2: Over Cabinet
// Channel 3: Sink
// Channel 4: UV

//////////////////////////////////////////////////////////////////////////
// V2.02  board
//////////////////////////////////////////////////////////////////////////
int BoardVersion = 3;

// D0/D1 - I2C only
// D2 & D3 only pins with PWM support.
// A4 = Channel 1 (Under Cambinet Lights)
// A5 = Channel 2 (White LED Strips)
// RX = Channel 3 (Install specific - Skin strips, Microwave LED strip etc)
// TX = Channel 4 (UV)
int lamps[] = { A4, A5, RX, TX };
int maxLamps = 4;

// User panel switch
// Switch input
int switchPin = D6;
// Switch LED.
int switchLed = D7;

//int currentSensorPin = WKP;

int vInPin = A3;
OneWire ds = OneWire(A2);  // 1-wire signal on pin D4
int lightLevelPin = A1;
int pirPin = A0;



//byte sensor1[] = {0x28, 0xD6, 0xA9, 0x4E, 0x07, 0x00, 0x00, 0x27};
// onboard temperature sensor address.
byte sensor[8];

Adafruit_INA219 ina219;

//////////////////////////////////////////////////////////////////////////

int on(String command);
int dim(String command);
int off(String command);
int neoPixelsOn(String args);

int lamp = 0;

// State:
// 0: Off
// 1: Dim
// 2: Bright
int desiredState = 1; // power up dimmed.
int currentState = -1;

volatile bool buttonPressed = false;
volatile bool pirTriggered = false;

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 4
#define PIXEL_TYPE WS2812B
#define PIXEL_PIN D5

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// Monitoring variables
float temperatureCelsius = 0;
double currentMilliAmps = 0;
double voltsIn = 0; // 5V rail monitor.
double supplyVoltage = 0; // the 12V in (monitored via current sensor)

// How many seconds to wait before turning off the lights.
double lightsOffInSeconds = 60;

// Measurement timer. Every n seconds.
//Timer takeMeasurementsTimer(10000, takeMeasurements);

//Timer dimLightsTimer(1000, dimLightsCheck);

void setup() {
    
    // Do not set A0 to analog in as it's not 5v tollerant in analog node.
    pinMode(A0, OUTPUT);
    
    for (int i=0; i< maxLamps; i++) {
        pinMode(lamps[i], OUTPUT);
        digitalWrite(lamps[i], false);
    }
    
    Particle.function("on", on);
    Particle.function("dim", dim);
    Particle.function("off", off);
    Particle.function("neoPixelsOn",neoPixelsOn);
    
    // Initialize the INA219.
    // By default the initialization will use the largest range (32V, 2A).  However
    // you can call a setCalibration function to change this range (see comments).
    ina219.begin();
    
    //pinMode(currentSensorPin, INPUT);
    pinMode(vInPin, INPUT);
    pinMode(lightLevelPin, INPUT);
    pinMode(pirPin, INPUT);
    pinMode(switchPin, INPUT_PULLUP);
    pinMode(switchLed, OUTPUT);
    digitalWrite(switchLed, false);
    
    // interrupt on the falling edge of A7 (switch pulled low)
    attachInterrupt(switchPin, buttonPressedIsr, FALLING);
    
    // TOOD: attachInterrup to PIR pin
    attachInterrupt(pirPin, pirTriggeredIsr, RISING);

    Particle.publish("Status", "Kitchen Lights Conroller. Version: 0.3.3, Board Version: " + String(BoardVersion));
    Particle.publish("Version", "0.3.3");
    
    // Setup NeoPixel LED (strip).
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    
    if (BoardVersion>=2) {
        listTemperatureSensors();
    }
    
    //takeMeasurementsTimer.start();
    //dimLightsTimer.start();
}

int loopCounter = 0;

void loop() {
    
    // Enable the switch LED now set-up is complete.
    digitalWrite(switchLed, true);
    delay(50);
    
    // Turn the switch LED off whilst processing.
    // So if loop failes to be re-called the LED will be left off
    // indicating a problem.
    digitalWrite(switchLed, false);
    
    loopCounter++;
    if (loopCounter > 1200) {
        loopCounter = 0;
        takeMeasurements();
    }
    
    dimLightsCheck();

    // Handle button press.
    if (buttonPressed) {
        desiredState = currentState + 1;
        if (desiredState > 2) {
            desiredState = 0;
        }
        
        // force a debounce delay then clear the indicator.
        delay(300);
        
        // Default, switch lights off 10 minutes after user switched on.
        if (lightsOffInSeconds<600) {
            lightsOffInSeconds = 600; 
        }
        
        Particle.publish("Status", "Button pressed.", 60, PRIVATE);
        buttonPressed = false;
    }
    
    if (pirTriggered) {
        // PIR means movement detected so don't turn off the lights!
        // TODO: start timer to switch off the lights if no further PIR
        // and the user button not pressed.
        setDesiredState(currentState + 1);
        
        // force a debounce delay then clear the indicator.
        delay(500);
        
        // Default, switch lights off  2 minutes after user switched on.
        if (lightsOffInSeconds<60) {
            lightsOffInSeconds = 60; 
        }
        
        Particle.publish("Status", "PIR sensor triggered.", 60, PRIVATE);
        pirTriggered = false;
    }
    
    setState();
}

// *******************************************************
// State management for lights.
// *******************************************************
void setDesiredState(int state) {
    desiredState = state;
    if (desiredState > 2) {
        desiredState = 2;
    }
    
    if (desiredState < 0) {
        desiredState = 0;
    }
}

void setState() {
    if (currentState != desiredState) {
        
        switch (desiredState) {
            case 0:
                off("");
                break;
            case 1:
                dim("");
                break;
            case 2:
                on("");
                break;
        }
        
        currentState = desiredState;
    }
}

// Timer routine to check and see if it's time to dim the lights.
// Called every second by the timer and checks to see if it 
// is time for the lights to be dimmed.
void dimLightsCheck() {
    lightsOffInSeconds-=0.05;
    if (lightsOffInSeconds <= 0) {
        dimLightsOnTimer();
        lightsOffInSeconds = 0;
    }    
}

void dimLightsOnTimer() {
    // ignore if the lights are currently off.
    if (currentState == 0) {
        return;
    }
    
    // Dim the lights one stage before actually turning off.
    setDesiredState(currentState-1);
    
    Particle.publish("Status", "Lights off timeout. Requesting state: " + String(currentState-1), 60, PRIVATE);
    
    // Allow 60 seconds between states 
    // Full -- initial timeout/no activity -> dim -> 60s timeout -> off)
    // dim -- initial timeout/no activity -> off)
    lightsOffInSeconds = 60;
}

// *******************************************************
// Take temperature, current and Photon VIn (5v rail) measurements.
// Called by timer every n seconds.
// *******************************************************
void takeMeasurements() {
    if (BoardVersion >= 2) {
        // Read the temperature.
        // Show the temperature from the first sensor.
        showTemperature(sensor);
        showCurrent();
        readVin();
        
        Particle.publish("senml", "{e:[{'n':'boardTemperature','v':'" + String(temperatureCelsius) + "'},{'n':'current','v':'" + String(currentMilliAmps) + "'},{'n':'supplyVoltage','v':'" + String(supplyVoltage) + "'},{'n':'lightState','v':'" + String(currentState) + "'},{'n':'vin','v':'" + String(voltsIn) + "'}]}", 60, PRIVATE);
    }
}

void showCurrent () {
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;

  float shuntvoltagemV = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V(); // at VIN- pin.
  // Adafruit library assumes 0.1R. Kitchen lights are fitted with a 0R01 resistor.
  current_mA = (ina219.getCurrent_mA() * 10);
  loadvoltage = busvoltage + (shuntvoltagemV / 1000);
  
  currentMilliAmps = current_mA;
  supplyVoltage = loadvoltage;
  
  //Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  //Serial.print("Shunt Voltage: "); Serial.print(shuntvoltagemV); Serial.println(" mV");
  //Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  //Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  //Serial.println("");
}

void listTemperatureSensors() {
  //Serial.println("------------------------------------------");
  //Serial.println("Sensors Discovered:");
  
  int sensorNumber = 0;
    
  do {   
    byte addr[8];
    
    if ( !ds.search(addr)) {
      ds.reset_search();
      return;
    }
  
    //Serial.print("ROM =");
    byte i;
    for( i = 0; i < 8; i++) {
      //Serial.write(' ');
      //Serial.print(addr[i], HEX);
      sensor[i] = addr[i];
    }
  
    if (OneWire::crc8(addr, 7) != addr[7]) {
        Serial.println("CRC is not valid!");
        return;
    } 
    
    // the first ROM byte indicates which chip
    switch (addr[0]) {
      case 0x10:
        Particle.publish("Status", "Found  DS18S20 ");
        break;
      case 0x28:
        Particle.publish("Status", "Found  DS18B20 :-) ");
        break;
      case 0x22:
        Particle.publish("Status", "Found  DS1822");
        break;
      default:
        Particle.publish("Status", "Found non a DS18x20 family device ");
        return;
    } 
  } while (true);
}

void showTemperature(byte sensorAddress[]) {
    byte i;
    byte present = 0;
    byte data[12];

    //  showAddress(sensorAddress); 

    // Start conversion.  
    ds.reset();
    ds.select(sensorAddress);
    ds.write(0x44);

    delay(1000);   // Delay to ensure conversion has happened. This might be update 750ms for 12bit. 375 (11 bit), 187 (10bit), 93 (9 bit)

    // Read conversion
    present = ds.reset();
    ds.select(sensorAddress);    
    ds.write(0xBE);         // Read Scratchpad
  
      // Read data
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
        data[i] = ds.read();
    }
    //showData( present, data);
  
    //Serial.print(" CRC=");
    //Serial.print(OneWire::crc8(data, 8), HEX);
    //Serial.println();
  
    temperatureCelsius = computeTemperature(data);
    
    //Particle.publish("Status", "Temperature read as " + String(temperatureCelsius));

    return;
}

float computeTemperature(byte data[]) {
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];

  byte cfg = (data[4] & 0x60);
  // at lower res, the low bits are undefined, so let's zero them
  if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
  else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
  else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  //// default is 12 bit resolution, 750 ms conversion time
    
  float celsius;
  celsius = (float)raw / 16.0;
  return celsius;
}

void readVin() {
    int vInAdc = analogRead(vInPin);
    // convert to ADC bits to millivolts
    // then x2 as it's a potential divider.
    voltsIn = (vInAdc * 0.8 * 2);
}

// *******************************************************
// Neopixel functions
// *******************************************************

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
    //for(i=0; i<4; i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

// *******************************************************
// Particle function functions (i.e. Internet exposed
// *******************************************************)

int on(String args) {
    
    if (args == "UNDER") {
        // Channel 1
        setLamp(0, true);
        Particle.publish("Status", "Under Lights On");
        return 0;
    } else if (args == "OVER") {
        // Channel 2
        setLamp(1, true);
        Particle.publish("Status", "Over Lights On");
        return 1;
    } else if (args == "SINK") {
        // Channel 3
        setLamp(2, true);
        Particle.publish("Status", "Sink Lights On");
        return 2;
    } else if (args == "UV") {
        // Channel 4
        setLamp(3, true);
        Particle.publish("Status", "UV Lights On");
        return 3;
  } else {
        // Generic "On" command.
        // Set Under, Over and sink to be on
        // UV to off.
        setLamp(0, true);
        setLamp(1, true);
        setLamp(2, true);
        setLamp(3, false);
        
        for(int pixel=0; pixel<strip.numPixels(); pixel++) {
            // Blue - 100%
            strip.setColorDimmed(pixel, 0, 0, 255, 255);
            //strip.setPixelColor(pixel, 0));
        }
        strip.show();
        
        Particle.publish("Status", "Lights On");
        
        return 200;
    }
}

int dim(String args) {
    if (args == "OVER") {
        setLamp(0, false);
        setLampDimmed(1, 40);
        setLamp(2, false);
        setLamp(3, false);
        Particle.publish("Status", "Over Lights Dimmed");
        return 1;
    } else {
        setLampDimmed(0, 40);
        setLampDimmed(1, 40);
        setLamp(2, false);
        setLamp(3, false);
        
        for(int pixel=0; pixel<strip.numPixels(); pixel++) {
            // Blue - 100%
            strip.setColorDimmed(pixel, 0, 255, 255, 128);
            //strip.setPixelColor(pixel, 0));
        }
        strip.show();
        
        Particle.publish("Status", "Lights Dimmed");
        return 2;
    }

    return 40;
}

int off(String args) {
    for (int i=0; i<maxLamps; i++) {
        setLamp(i, false);
    }
    
    for(int pixel=0; pixel<strip.numPixels(); pixel++) {
        strip.setColorDimmed(pixel, 0, 0, 0, 0);
        //strip.setPixelColor(pixel, 0));
    }
    strip.show();
    
    Particle.publish("Status", "All Lights Off");

    return 0;
}

int neoPixelsOn(String args) {
    rainbow(200);
    
    Particle.publish("Status", "Neopixel lights on");
    
    return 0;
}

// *******************************************************
// Helpers
// *******************************************************

void setLamp(int lamp, bool state) {
    digitalWrite(lamps[lamp], state);
}

void setLampDimmed(int lamp, int brightness) {
    analogWrite(lamps[lamp], brightness);
}

// *******************************************************
// Interrup service routines.
// *******************************************************
void buttonPressedIsr() {
    buttonPressed = true;
}

// ISR for PIR sensor.
void pirTriggeredIsr() {
    pirTriggered = true;
}