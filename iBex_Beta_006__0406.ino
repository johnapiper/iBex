

//iBex 1.
//Firmware Revision: Beta 006 (0406).


//Config
#define DIAGNOSTICS true // Change this to see Iridium diagnostics in Debug Window.
#define IRIDIUM_SLEEP_PIN 25
#define Alt_Stage3Trigger 1000
#define Alt_Stage4Trigger 15000
#define Alt_Stage5Trigger 20000
//Delays
#define Delay_SensorRead 5000
#define Delay_SensorDebug 10000
#define Delay_Iridium 60000
#define Delay_IridiumSignalCheck 40000

#define Press_Sea 101325

#define PIN_AUDIO 5
#define PIN_LED 13
#define PIN_ANALOG_BATTERY A1

#define PIN_GPS_RX 8
#define PIN_GPS_TX 7
#define PIN_TNC_RX 12
#define PIN_TNC_TX 11

#define DELAY_MS_BETWEEN_XMITS 1250
#define GPS_TIMEOUT_TIME 45000      //number of milliseconds to wait between error transmissions if the GPS fails
#define GPS_MAX_COLLECTION_TIME 3000    //number of millis to wait while collecting the two GPS strings.
#define METERS_TO_FEET 3.2808399

//Anunciator Settings
#define DELAY_DAH 650
#define DELAY_DIT 200
#define DELAY_GAP 150





//Libraries
#include <SD.h>
#include "GPS.h"
#include "TMP102.h"
#include "BMP180.h"
#include <MS5xxx.h>
#include <Wire.h>

MS5xxx sensor(&Wire);

//Iridium
#include <IridiumSBD.h>
uint8_t buffer[200] = 
{ 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89 };
#define IridiumSerial Serial3
IridiumSBD modem(IridiumSerial,IRIDIUM_SLEEP_PIN);
static bool messageSent = false;

//APRS Lib
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include "TNC.h"
//APRS Defininitions
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
#define APRS_FIRMWARE_VERSION "2.1.7"
#define CONFIG_VERSION "PT0002"
#define CONFIG_PROMPT "\n# "

#define SER_DEBUG


//Full Scope Definitions.
int Stage;
int CycleNum;

long Time;
long Time_SensorRead;
long Time_SensorDebug;
long Time_Iridium;
long Time_IridiumSignalCheck;
long Time_Launch;
long Time_Stage1;
long Time_Stage2;
long Time_Stage3;
long Time_Stage4;
long Time_Stage5;
long Time_Stage6;
long Time_Stage7;
String Time_Failure="NULL";
String Failure_Reason="NULL";
char DataByte;
long Alt_Actual = 0;
long Alt_Ascent = 0;
long Press_Release = 0;
double MS5607_Pressure;
double MS5607_Temp;
double Lat1;
double Long1;

float SystemCheck_Pass = 0;
char Time_Stamp[21];
int Failure_ID;
String debug="NULL";
String debug3="NULL";
String debug_Last="NULL";
double debug2;

struct udtConfig {
  char Callsign[7];    //6 digit callsign + Null
  char CallsignSSID;
  char Destination[7];
  char DestinationSSID;    //Destination SSID
  char Path1[7];
  char Path1SSID;
  char Path2[7];
  char Path2SSID;

  unsigned int DisablePathAboveAltitude;    //the altitude to stop sending path.  If 0, then always send path defined.

  char Symbol;
  char SymbolPage;

  byte BeaconType;    //0=seconds-delay, 1=Speed Smart Beaconing, 2=Altitude Smart Beaconing, 3=Time Slots
  byte BeaconSimpleDelay;

  unsigned int BeaconAltitudeThreshLow;
  unsigned int BeaconAltitudeThreshHigh;
  unsigned long BeaconAltitudeDelayLow;
  unsigned long BeaconAltitudeDelayMid;
  unsigned long BeaconAltitudeDelayHigh;

  unsigned int BeaconSpeedThreshLow;
  unsigned int BeaconSpeedThreshHigh;
  unsigned long BeaconSpeedDelayLow;
  unsigned long BeaconSpeedDelayMid;
  unsigned long BeaconSpeedDelayHigh;

  byte BeaconSlot1;
  byte BeaconSlot2;

  byte AnnounceMode;    //0=None, 1=LED, 2=Audio, 3=LED+Audio

  byte GPSSerialBaud;    //1=300, 2=1200 3=2400 4=4800 5=9600 6=19200
  bool GPSSerialInvert;    //Invert the incoming serial string.
  byte GPSType;      //0=Generic NMEA, 1=UBlox

  char StatusMessage[41];
  bool StatusXmitGPSFix;
  bool StatusXmitBurstAltitude;
  bool StatusXmitBatteryVoltage;
  bool StatusXmitTemp;
  bool StatusXmitPressure;

  unsigned int CheckSum;    //sum of the callsign element.  If it doesn't match, then it reinitializes the EEPROM
};
udtConfig Config;

GPS GPSParser;    //Object that parses the GPS strings
TNC oTNC;            //Object that assembles the packets for the TNC and transmits them

unsigned long timeLastXmit;    //Keeps track of the timestamp of the last transmission
unsigned long iLastErrorTxMillis;    //keep track of the timestamp of the last "lost GPS" transmission

bool bHasBurst;
float fMaxAlt;

BMP180 Pressure;      //BMP180 pressure/temp sensor
TMP102 OAT;    //TMP102 sensor for outside air temp

int signalQuality = -999;
int err;


//Scripts
void setup() {
Startup_LibraryDefinitions();
//APRS_Setup(); //Commented out
Startup_IridiumModem();
Sensor_Setup();
Sensor_Read();
ConcatData();
System_Check();
}

void loop() {
Serial.println(DataByte);
TimeHandler();
switch (Stage) {
  case 1:
  Stage1();
  case 2:
  Stage2();
  case 3:
  Stage3();
  case 4:
  Stage4();
  case 5:
  Stage5();
  case 6:
  Stage6();
  case 7:
  Stage7();
}
if(Time > (Time_SensorRead+Delay_SensorRead)) {Sensor_Read();}
if(Time > (Time_SensorDebug+Delay_SensorDebug)) {Debug_ALLSensors();}
if(Time > (Time_IridiumSignalCheck+Delay_IridiumSignalCheck)) {Iridium_SignalCheck();}
if(Time > (Time_Iridium+Delay_Iridium)&&signalQuality>0) {Iridium();}
}
