/*

  Minimal setup for 1 DIO pin to be used for OneWire DS18B20


*/



#include <Wire.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <CRC32.h>


// Data wire is plugged into port 2 on the Arduino/
#define ONE_WIRE_BUS 3
#define TEMPERATURE_PRECISION 12

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress Thermometer1;

//result string
String stringResult;

char inByte = 0;         // incoming serial byte
void setup(void)
{
  pinMode(13, OUTPUT);

  // start serial port
  Serial.begin(19200);

  delay(800);
  // Start up the library
  sensors.begin();

  delay(50);
  // method 1: by index
  if (!sensors.getAddress(Thermometer1, 0)) Serial.println("Unable to find address for Device 0");
  delay(50);

  // set the resolution to 12 bit
  sensors.setResolution(Thermometer1, TEMPERATURE_PRECISION);
  delay(50);
}

void loop(void)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  //  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  delay(800);

  printTemperature(Thermometer1, 'A'); // UNTAGGED

  blinkLed(20, 30);

  delay(4000);


}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress, char tag)
{
  static char buf[20];
  uint32_t checksum;
  float tempC = sensors.getTempC(deviceAddress);
  buf[0] = tag;
  dtostrf(tempC, 7, 4, &buf[1]);
  stringResult = buf;
  int len = stringResult.length();
  checksum = CRC32::calculate(buf, len);
  Serial.print(buf);
  Serial.println(String(checksum, HEX));
}

void blinkLed(int iOn, int iOff)
{
  digitalWrite(13, HIGH);   // set the LED on
  delay(100);              // wait for a second
  digitalWrite(13, LOW);    // set the LED off
  delay(100);              // wait for a second
}

