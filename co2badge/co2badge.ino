/*
  CO2 Traffic light on MakeZurich 2020 badge using the SCD30 provided in the lab
  By: Michel Racic
  Date: October 29, 2020
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  This example prints the current CO2 level, relative humidity, and temperature in C.

  Hardware Connections:
  Attach MZ20 badge to computer using a USB cable.
  Connect SCD30 sensor to X1 (the right side) shitty addon port on the badge.
    Find the datasheet at https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/9.5_CO2/Sensirion_CO2_Sensors_SCD30_Datasheet.pdf
    Connect VDD to the first pin in upper row [VCC] (left to right)
    Connect GND to the first pin in the lower row
    Connect SCL to the second pin on the lower row
    Connect SDA to the second pin on the upper row

    ___
    |(SDC30)
    |
    |
    |SEL|
    |PWM|
    |RDY|                 (X1)
    |SDA|-------------------|
    |SCL|-\     -----[VCC][SDA][RX]
    |GND|--\---/-----[GND][SCL][TX]
    |VDD|---\-/             |
    ___      ---------------|

  Open Serial Monitor at 115200 baud.

  Possible enhancements:
  - Connect RDY PIN to a GPIO on the nano and use interrupt to trigger when data is ready
    instead of having a fixed duty cycle
*/

#include <Wire.h>

#include <Adafruit_NeoPixel.h>
#define LED_PIN 4
#define LED_COUNT 11
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30
SCD30 airSensor;
#define CO2_WARN_TRESHOLD 800
#define CO2_FULL_RED_TRESHOLD 1000
#define CO2_LED_MAPPING_MAX 1000
#define CO2_READING_DUTTY_CYCLE 500


void setup()
{
  // Start the serial console for the printouts
  Serial.begin(115200);
  // Initiate I2C communication
  Wire.begin();


  // Initialize the NeoPixel ring on the badge
  strip.begin();
  strip.show(); // all off
  strip.setBrightness(30); // max 255
  
  Serial.println("CO2 Traffic light on MakeZurich 2020 badge");
  
  if (airSensor.begin() == false)
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    while (1)
      ;
  }

  //The SCD30 has data ready every two seconds
}

void loop()
{
  if (airSensor.dataAvailable())
  {
    Serial.print("co2(ppm):");
    int currentCO2 = airSensor.getCO2();
    Serial.print(currentCO2);
    
    // Map the current CO2 value to the range of the 11 LEDs on the badge
    int n = map(currentCO2, 0, CO2_LED_MAPPING_MAX, 0, LED_COUNT);
    for (int i = 0; i < n; i++) {
      if(currentCO2 < CO2_WARN_TRESHOLD) {
        // Set to full green until we reach CO2_WARN_TRESHOLD
        strip.setPixelColor(i, strip.Color(0, 255, 0));
      } else if(currentCO2 < CO2_FULL_RED_TRESHOLD) {
        // Mapping green and red values oposite to get a transition between the colors for values
        // between CO2_WARN_THRESHOLD and CO2_FULL_RED_TRESHOLD
        int valred = map(currentCO2, CO2_WARN_TRESHOLD, CO2_FULL_RED_TRESHOLD, 0, 255);
        int valgreen = map(currentCO2, CO2_WARN_TRESHOLD, CO2_FULL_RED_TRESHOLD, 255, 0);
        strip.setPixelColor(i, strip.Color(valred, valgreen, 0));
      } else {
        // Set to full red as we breached the critical limit
        strip.setPixelColor(i, strip.Color(255, 0, 0));
      }
    }
    for (int j = n; j < LED_COUNT; j++) {
      //set remaining LEDs off
      strip.setPixelColor(j, strip.Color(0, 0, 0));
    }
    strip.show();

    Serial.print(" temp(C):");
    Serial.print(airSensor.getTemperature(), 1);

    Serial.print(" humidity(%):");
    Serial.print(airSensor.getHumidity(), 1);

    Serial.println();
  }

  delay(CO2_READING_DUTTY_CYCLE);
}
