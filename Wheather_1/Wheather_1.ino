/*
Change Log

 22/07/2012
 - Revert back to DS3231 RTC code using a DS3232 Freetronics chip on PINS A4 (SDA - Yellow) A5 (SCL - Blue)
 - Comment out Garage Temp measurement.

 25/07/2012
 - Implement a reset on Pumping Totals as the date rolls over.

30/7/2015
 - reduce program to suit new requirements.
 - RTC Hardware no longer needed

10/07/2018
 - physically remove RTC hardware

 
 */



#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Date and time functions using RTC connected via I2C and Wire lib

// Data wire is plugged into port 2 on the Arduino/
#define ONE_WIRE_BUS 6
#define TEMPERATURE_PRECISION 12

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress Thermometer1, Thermometer2, Thermometer3, Thermometer4;

unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change :
const long interval = 2000;           // interval at which to blink (milliseconds)

//result string
String stringResult;

char inByte = 0;         // incoming serial byte

// variables for sensing pump
int INpin = 3;
volatile int Pstate = LOW;
volatile int LastPstate = LOW;
long PstartTime;
long PelaspedTime;
int PumpMessage = LOW;

int LastDay = 0;

void setup(void)
{
  // setup Pump sensing DIO
  pinMode(INpin, INPUT);
  digitalWrite(INpin, HIGH);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);

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

  delay(50);
}

void loop(void)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    //  Serial.print("Requesting temperatures...");
    sensors.requestTemperatures();
    delay(800);
    //  Serial.println("DONE");
    // print the device information
    //  printTemperature(Thermometer1, 'G'); // On board
    printTemperature(Thermometer1, 'A'); // UNTAGGED

    /*
    // if we get a valid byte, read date-time adjust parameters.
     if (Serial.available() > 0) {
     //    processAdjsutDT();
     }
     */
  }
  blinkLed(20, 30);
  CheckPumpSensor();
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress, char tag)
{
  noInterrupts();
  float tempC = sensors.getTempC(deviceAddress);
  interrupts();

  static char buf[30];
  buf[0] = tag;
  dtostrf(tempC, 5, 2, &buf[1]);
  stringResult = buf;
  int len = stringResult.length();
  int icheck = getCheckSum(buf, len);
  sprintf(&buf[len], "%2.2X", icheck);
  Serial.println(buf);
}

void blinkLed(int iOn, int iOff)
{
  digitalWrite(13, HIGH);   // set the LED on
  delay(iOn);              // wait for a second
  digitalWrite(13, LOW);    // set the LED off
  delay(iOff);              // wait for a second
}


// Calculates the checksum for a given string
// returns as integer
char getCheckSum(char buf[], int iStrLen) {
  int i;
  char XOR;
  char c;
  // Calculate checksum ignoring any $'s in the string
  for (XOR = 0, i = 0; i < iStrLen; i++) {
    c = (unsigned char)buf[i];
    if (c == '\n') break;
    XOR ^= c;
  }
  return XOR;
}

// Pum sensing INTERUPT code
void PumpSensor()
{
  Pstate = digitalRead(INpin);
  if (Pstate == HIGH)
  {
    PstartTime = millis();

  }
  else
  {
    PelaspedTime = millis() - PstartTime;
    PumpMessage = HIGH;
  }
}

// Pump sensing polling code
void CheckPumpSensor()
{
  Pstate = digitalRead(INpin);
  digitalWrite(12, Pstate);
  if (LastPstate != Pstate)
  {
    LastPstate = Pstate;
    if (Pstate == HIGH)
    {
      PstartTime = millis();

    }
    else
    {
      PelaspedTime = millis() - PstartTime;
      PumpMessage = HIGH;
    }
    if (PumpMessage == HIGH)
    {
      // Construct a Pump Status messsage
      printPump();
    }
  }
}

// function to print the Pump information
void printPump()
{
  int iDuration;
  char buf[20];

  stringResult = "P";
  iDuration = PelaspedTime / 1000;
  stringResult += iDuration;

  buf[0] = 'P';
  sprintf(&buf[1], "%i", iDuration);
  stringResult = buf;
  int len = stringResult.length();
  int icheck = getCheckSum(buf, len);
  sprintf(&buf[len], "%2.2X", icheck);
  Serial.println(buf);
  PumpMessage = LOW;
}


