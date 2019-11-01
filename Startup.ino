void Title() {
  debug=("====================  iBex  ====================");
  Debug();
  DebugTerm();
  debug="";
  Debug();
  DebugTerm();
  debug="iBex Project Firmware: Beta_006_0406";
  Debug();
  DebugTerm();
  debug="";
  Debug();
  DebugTerm();
  EEPROM_CycleNum();
  debug=("====================  iBex  ====================");
  Debug();
  DebugTerm();
  delay(5000);
}





void Startup_LibraryDefinitions() {
  TimeHandler();
  Serial.begin(115200);
  Title();
  debug=("");
  Debug();
  DebugTerm();

//Debug Boot  
  debug = "Serial Started on 115200 baud";
  Debug();
  DebugTerm();
  debug = "Booting in... ~3 Seconds";
  Debug();
  DebugTerm();
  delay(500);
  debug = "Booting in... ~2 Seconds";
  Debug();
  DebugTerm();
  delay(1000);
  debug = "Booting in... ~1 Seconds";
  Debug();
  DebugTerm();
  delay(3000);
  debug = "";
  Debug();
  DebugTerm();
return;
}

void Startup_IridiumModem () {
// Setup the Iridium modem
  IridiumSerial.begin(19200);
  debug="=============  Iridium Modem Setup  ====================";
  DebugTerm();
  Debug();
  DebugTerm();
  DebugTerm();
  modem.setPowerProfile(IridiumSBD::DEFAULT_POWER_PROFILE);
  modem.adjustSendReceiveTimeout(10);
  if (modem.begin() != ISBD_SUCCESS)
  {
    debug=("Couldn't begin modem operations.");
    Debug();
    DebugTerm();
    return;
  }
  debug="====================  Iridium Setup COMPLETE  ====================";
  DebugTerm();
  Debug();
  DebugTerm();
  DebugTerm();
return;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Sensor_Setup() {
  debug="Connecting to Sensor Modules: 76%";
  Debug();
  DebugTerm();
  delay(5000);
  debug="Connecting to Sensor Modules: 100%";
  Debug();
  DebugTerm();
  if(sensor.connect()>0) {
    debug=("Error connecting to MS5607...");
    Debug();
    DebugTerm();
    delay(500);
    Sensor_Setup();
}
else {
    debug=("Succsussfully connected to MS5607");
    Debug();
    DebugTerm();
    delay(500);
    debug=("Altitude Calibrated to Pressure: ");
    Sensor_Read();
    Press_Release=MS5607_Pressure;
    debug2=(Press_Release);
    Debug();
    Debug2();
    DebugTerm();
    debug=("");
    Debug();
    DebugTerm();
}

return;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EEPROM_CycleNum() {
CycleNum = EEPROM.read(100);
CycleNum++;
EEPROM.write(100, CycleNum);
    debug=("");
    Debug();
    DebugTerm();
  debug="Cycle Number: ";
  debug2=CycleNum;
  Debug();
  Debug2();
  DebugTerm();
    debug=("");
    Debug();
    DebugTerm();
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void APRS_Setup() {

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_AUDIO, OUTPUT);

  oTNC.initKISS(PIN_TNC_RX, PIN_TNC_TX);      //Define this as a KISS-connected controller (i.e. this Arduino does NOT generate its own tones)

  Serial.println(F("Arduino_APRS Flight Computer"));
  Serial.print(F("iBex: APRS Module Firmware Version: "));
  Serial.println(APRS_FIRMWARE_VERSION);


  //Init some variables
  fMaxAlt = 0;
  bHasBurst = false;
  timeLastXmit = 0;


  getConfigFromEeprom();

  annunciate('k');

  //init the I2C devices
  Serial.println(F("Init'ing BMP180 sensor"));
  if (!Pressure.begin()) {
    Serial.println(F(" Could NOT init!"));
  }
  
  Serial.println(F("Init'ing TMP102 sensor"));
  OAT.begin();
  
  //Check to see if we're going into config mode
  byte byTemp;
  while (millis() < 5000) {
    if (Serial.available() > 0) {
      // read the incoming byte:
      byTemp = Serial.read();

      if (byTemp == '!') {
        doConfigMode();
      }
    }
  }

  getConfigFromEeprom();


  //Send out an initial packet announcing itself.
  oTNC.xmitStart(Config.Destination, Config.DestinationSSID, Config.Callsign, Config.CallsignSSID, Config.Path1, Config.Path1SSID, Config.Path2, Config.Path2SSID, true);
  oTNC.xmitString(">Project Traveler ArduinoTrack Controller v");
  oTNC.xmitString(APRS_FIRMWARE_VERSION);
  oTNC.xmitString(" Initializing...");
  oTNC.xmitEnd();

  //see if we're using a uBlox GPS, and if so, init the GPS
  if (Config.GPSType == 1) {
    //init the GPS into high altitude mode (Dynamic Model 6 � Airborne < 1g)
    initUblox();    //will continually retry this operation until its sucessful
  }
  
  iLastErrorTxMillis = millis();      //set a starting time for the potential error messages
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
