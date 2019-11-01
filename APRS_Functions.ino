/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void collectGPSStrings() {

#ifdef SER_DEBUG
Serial.println(F("Collecting GPS..."));
#endif

  SoftwareSerial GPS(PIN_GPS_RX, PIN_GPS_TX, Config.GPSSerialInvert);    //A True at the end indicates that the serial data is inverted.

  //figure out the baud rate for the data coming in
  switch (Config.GPSSerialBaud) {
    case 0x01:
      GPS.begin(300);
      break;
    case 0x02:
      GPS.begin(1200);
      break;
    case 0x03:
      GPS.begin(2400);
      break;
    case 0x04:
      GPS.begin(4800);
      break;
    case 0x05:
      GPS.begin(9600);
      break;
    case 0x06:
      GPS.begin(19200);
      break;
  }

  GPSParser.clearInputBuffer();
  GPSParser.ClearSentenceFlags();      //clear out the temporary flags to indicate that the new sentences have come in


  //keep track of how long we can listen to the GPS
  unsigned long ulUntil = millis() + GPS_MAX_COLLECTION_TIME;


  while (millis() < ulUntil ) {
    //need to continue looping even if the data isn't coming in.

    //see if there's some new GPS data available
    if (GPS.available()) {
      byte c = GPS.read();

      GPSParser.addChar(c);

      //check the sentence flags to see if both RMC and GGA's have been received in this session
      if (GPSParser.GotNewRMC() && GPSParser.GotNewGGA()) {
        //we got new GGA and RMC strings - exit out now rather than waiting the whole alloted period.
        return;
      }
    }
  }

#ifdef SER_DEBUG
Serial.println(F("Stop collection. Probably timed out."));
#endif
  return;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void annunciate(char c) {
  //send an anunciator via LED and/or buzzer, depending on config

  switch (c) {
  case 'c':
    //Used when entering configuration mode
    audioTone(DELAY_DAH);
    delay(DELAY_GAP);
    audioTone(DELAY_DIT);
    delay(DELAY_GAP);
    audioTone(DELAY_DAH);
    delay(DELAY_GAP);
    audioTone(DELAY_DIT);
    break;
  case 'e':
    //single chirp
    audioTone(DELAY_DIT);
    break;
  case 'g':
    //Used during complete loss of GPS signal
    audioTone(DELAY_DAH);
    delay(DELAY_GAP);
    audioTone(DELAY_DAH);
    delay(DELAY_GAP);
    audioTone(DELAY_DIT);
    break;
  case 'i':
    //double chirp
    audioTone(DELAY_DIT);
    delay(DELAY_GAP);
    audioTone(DELAY_DIT);
    break;
  case 'k':
    //Initial "OK"
    audioTone(DELAY_DAH);
    delay(DELAY_GAP);
    audioTone(DELAY_DIT);
    delay(DELAY_GAP);
    audioTone(DELAY_DAH);
    break;

  case 'l':
    //Used during loss of GPS lock
    audioTone(DELAY_DIT);
    delay(DELAY_GAP);
    audioTone(DELAY_DAH);
    delay(DELAY_GAP);
    audioTone(DELAY_DIT);
    delay(DELAY_GAP);
    audioTone(DELAY_DIT);
    break;

  case 'o':
    audioTone(DELAY_DAH);
    delay(DELAY_GAP);
    audioTone(DELAY_DAH);
    delay(DELAY_GAP);
    audioTone(DELAY_DAH);
    break;

  case 'w':
    audioTone(DELAY_DIT);
    delay(DELAY_GAP);
    audioTone(DELAY_DAH);
    delay(DELAY_GAP);
    audioTone(DELAY_DAH);
    break;
    
  case 'x':
    audioTone(DELAY_DAH);
    delay(DELAY_GAP);
    audioTone(DELAY_DIT);
    delay(DELAY_GAP);
    audioTone(DELAY_DIT);
    delay(DELAY_GAP);
    audioTone(DELAY_DAH);
    break;
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void audioTone(int length) {

  //The Config parameter Config.AnnounceMode determines what this function does.  If bit 0 is set, then the LED will flash.
  // If the bit 1 is set, then it will pulse the audio annunciator.  Both or neither bits can also be set and behave accordingly.
  if (Config.AnnounceMode & 0x01) {
    digitalWrite(PIN_LED, HIGH);
  }

  for (int i = 0; i<length; i++) {
    if (Config.AnnounceMode & 0x02) {
      digitalWrite(PIN_AUDIO, HIGH);
    }
    delayMicroseconds(200);

    if (Config.AnnounceMode & 0x02) {
      digitalWrite(PIN_AUDIO, LOW);
    }
    delayMicroseconds(200);
  }

  if (Config.AnnounceMode & 0x01) {
    digitalWrite(PIN_LED, LOW);
  }

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void initUblox(void) {
  bool bSuccess = false;

  while(!bSuccess) {
    annunciate('e');    //chirp
    bSuccess = ubloxSendUBX();
  }
  annunciate('i');    //double chirp for success
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ubloxSendUBX() {
  //start up the GPS serial port - always use 9600 to init the UBlox
  SoftwareSerial GPS(PIN_GPS_RX, PIN_GPS_TX, Config.GPSSerialInvert);    //A True at the end indicates that the serial data is inverted.
  GPS.begin(9600);


  byte setdm6[] = {0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
                   0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC };

  byte ackByteID = 0;
  byte ackPacket[10];


  //calculate a response checksum to verify that the config was sent correct.
  // Construct the expected ACK packet
  ackPacket[0] = 0xB5; // header
  ackPacket[1] = 0x62; // header
  ackPacket[2] = 0x05; // class
  ackPacket[3] = 0x01; // id
  ackPacket[4] = 0x02; // length
  ackPacket[5] = 0x00;
  ackPacket[6] = setdm6[2]; // ACK class
  ackPacket[7] = setdm6[3]; // ACK id
  ackPacket[8] = 0; // CK_A
  ackPacket[9] = 0; // CK_B

  // Calculate the checksums
  for (byte i=2; i<8; i++) {
    ackPacket[8] = ackPacket[8] + ackPacket[i];
    ackPacket[9] = ackPacket[9] + ackPacket[8];
  }

  Serial.println(F("Init uBlox"));
  //send the config to the GPS
  GPS.flush();
  GPS.write(0xFF);
  delay(500);

  for (byte i=0; i<44; i++) {
    GPS.write(setdm6[i]);
  }

  //keep track of how long we can listen to the GPS
  unsigned long ulUntil = millis() + 3000;

  while (millis() < ulUntil ) {
    // Test for success
    if (ackByteID > 9) return true;    //we had all 9 bytes come back through - valid response!!!


    // Make sure data is available to read
    if (GPS.available()) {
      byte c = GPS.read();

      // Check that bytes arrive in sequence as per expected ACK packet
      if (c == ackPacket[ackByteID]) {
        ackByteID++;
      } else {
        ackByteID = 0; // Reset and look again, invalid order
      }
    }
  }
  return false;    //timed out
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getConfigFromEeprom() {
  for (unsigned int i=0; i<sizeof(Config); i++) {
    *((char*)&Config + i) = EEPROM.read(i);
  }

  //Check to see if the EEPROM appears to be valid
  unsigned int iCheckSum = 0;
  for (int i=0; i<7; i++) {
    iCheckSum += Config.Callsign[i];
  }

  Serial.println(F("Read EEPROM"));
  Serial.print(F("Callsign: "));
  Serial.println(Config.Callsign);

  if (iCheckSum != Config.CheckSum) {
    Serial.println(F("Checksums don't match.  Resetting to defaults."));

    //we do NOT have a match - reset the Config variables
    setDefaultConfig();
  }

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setDefaultConfig() {
  strcpy(Config.Callsign, "N0CALL");
  Config.CallsignSSID = '0';
  strcpy(Config.Destination, "APRS  ");
  Config.DestinationSSID = '0';
  strcpy(Config.Path1, "      ");
  Config.Path1SSID = '0';
  strcpy(Config.Path2, "      ");
  Config.Path2SSID = '0';
  Config.DisablePathAboveAltitude = 0;
  Config.Symbol = 'O';    //letter O for balloons
  Config.SymbolPage = '/';
  Config.BeaconType = 0;
  Config.BeaconSimpleDelay = 30;
  Config.BeaconSpeedThreshLow = 20;
  Config.BeaconSpeedThreshHigh = 50;
  Config.BeaconSpeedDelayLow = 300;
  Config.BeaconSpeedDelayMid = 60;
  Config.BeaconSpeedDelayHigh = 120;
  Config.BeaconAltitudeThreshLow = 5000;
  Config.BeaconAltitudeThreshHigh = 20000;
  Config.BeaconAltitudeDelayLow  = 30;
  Config.BeaconAltitudeDelayMid  = 60;
  Config.BeaconAltitudeDelayHigh = 45;
  Config.BeaconSlot1 = 15;
  Config.BeaconSlot2 = 45;
  strcpy(Config.StatusMessage, "iBEX_APRS_");
  Config.StatusXmitGPSFix = 1;
  Config.StatusXmitBurstAltitude = 1;
  Config.StatusXmitBatteryVoltage = 1;
  Config.StatusXmitTemp = 1;
  Config.StatusXmitPressure = 1;
  Config.GPSSerialBaud = 5;    //1=300, 2=1200, 3=2400, 4=4800, 5=9600, 6=19200
  Config.GPSSerialInvert = 0;    //Invert the incoming signal
  Config.GPSType = 1;      //0=Generic NMEA, 1=UBlox
  Config.AnnounceMode = 1;

  Config.CheckSum = 410;    //Checksum for N0CALL

  writeConfigToEeprom();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void writeConfigToEeprom() {
  for (unsigned int i=0; i<sizeof(Config); i++) {
    EEPROM.write(i, *((char*)&Config + i));
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void doConfigMode() {
  byte byTemp;

  Serial.println(F("ArduinoTrack Flight Computer"));
  Serial.print(F("Firmware Version: "));
  Serial.print(APRS_FIRMWARE_VERSION);
  Serial.print(F("   Config Version: "));
  Serial.println(CONFIG_VERSION);
  Serial.print(CONFIG_PROMPT);

  delay(750);
  annunciate('c');

  while (byTemp != 'Q') {
    if (Serial.available()) {
      byTemp = Serial.read();

      if (byTemp == '!') {
        Serial.println(F("ArduinoTrack Flight Computer"));
        Serial.print(F("Firmware Version: "));
        Serial.print(APRS_FIRMWARE_VERSION);
        Serial.print(F("   Config Version: "));
        Serial.println(CONFIG_VERSION);
        Serial.print(CONFIG_PROMPT);
      }


      if (byTemp == 'R') {
        getConfigFromEeprom();    //pull the configs from eeprom
        sendConfigToPC();

        Serial.write(CONFIG_PROMPT);
      }

      if (byTemp == 'W') {
        //take the incoming configs and load them into the Config UDT

        Serial.println(F("Entering config write mode..."));

        if (getConfigFromPC()) {
          Serial.println(F("Done reading in configuration data."));

          writeConfigToEeprom();
          Serial.println(F("Written config to eeprom."));

          annunciate('w');
        } else {
          //something failed during the read of the config data
          Serial.println(F("Failure to read in configuration data..."));
        }

        Serial.write(CONFIG_PROMPT);
      }
      
      if (byTemp == 'D') {
        //used to reset the ArduinoTrack back to N0CALL defaults
        Serial.println(F("Resetting to defaults (N0CALL)"));
        setDefaultConfig();        
        annunciate('w');
        Serial.write(CONFIG_PROMPT);
      }
      
      if (byTemp == 'E') {
        //exercise mode to check out all of the I/O ports
        
        Serial.println(F("Exercising the ArduinoTrack"));
        
        Serial.println(F("Testing annunciators"));
        Config.AnnounceMode = 0x03;    //temporarily set the announce mode to both
        annunciate('x');
        
        //check the IO pins
        Serial.println(F("Pin 4"));
        pinMode(4, OUTPUT);
        digitalWrite(4, HIGH);
        delay(1000);
        digitalWrite(4, LOW);
        pinMode(4, INPUT);

        Serial.println(F("Pin 6"));
        pinMode(6, OUTPUT);
        digitalWrite(6, HIGH);
        delay(1000);
        digitalWrite(6, LOW);
        pinMode(6, INPUT);        
        
        Serial.println(F("Pin 10"));
        pinMode(10, OUTPUT);
        digitalWrite(10, HIGH);
        delay(1000);
        digitalWrite(10, LOW);
        pinMode(10, INPUT);  

        //Analog A2
        Serial.println(F("Pin A2"));
        pinMode(16, OUTPUT);
        digitalWrite(16, HIGH);
        delay(1000);
        digitalWrite(16, LOW);
        pinMode(16, INPUT);          
        
         //Analog A3
        Serial.println(F("Pin A3"));
        pinMode(17, OUTPUT);
        digitalWrite(17, HIGH);
        delay(1000);
        digitalWrite(17, LOW);
        pinMode(17, INPUT);   
  
        collectGPSStrings();   //check the GPS  
  
        double airTemp;    //inside air temp
        double airPressure;    //millibars
  
  
        char status = Pressure.startTemperature();

        if (status != 0) {
          // Wait for the measurement to complete:
          delay(status);
    
          status = Pressure.getTemperature(airTemp);
          if (status != 0) {
            status = Pressure.startPressure(3);
    
            if (status != 0)
            {
              // Wait for the measurement to complete:
              delay(status);
    
              status = Pressure.getPressure(airPressure,airTemp);
              if (status == 0)
              {
                //we had some sort of problem with getting the air pressure - set it to zero
                airPressure = 0.0;
              }
            }
          } else {
            //problem getting air temp
            airTemp = 0.0;
            airPressure = 0.0;
          }
        }
    
        Serial.print(F("IAT: "));
        Serial.println(airTemp);
        Serial.print(F("Pressure: "));
        Serial.println(airPressure);   
        
        //Read external temp
        airTemp = 0.0;
        
        Serial.print(F("OAT: "));
        status = OAT.getTemperature(airTemp);
        if (status == 0) {
          Serial.println("n/a");
        } else {
          Serial.println(airTemp);
        }
        
        
        
        Serial.write(CONFIG_PROMPT);
      }
    }
  }
  Serial.println(F("Exiting config mode..."));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void readConfigParam(char *szParam, int iMaxLen) {
  byte c;
  int iSize;
  unsigned long iMilliTimeout = millis() + 1000;    //wait up to 1 second for this data

  for (iSize=0; iSize<iMaxLen; iSize++) szParam[iSize] = 0x00;    //load the array with nulls just in case we don't find anything
  iSize = 0;    //reset to start counting up for real

  //Serial.println("W: ");
  while (millis() < iMilliTimeout) {

    if (Serial.available()) {
      c = Serial.read();
      //Serial.write(c);    //echo it back out

      if (c == 0x09 || c == 0x04) {
        //this is the end of a data set

        //Serial.println("::");
        //Serial.println(szParam);
        return;
      }
      if (iSize < iMaxLen) {
        //only add to the return array IF there's room.  Even if there's not room, continue to parse the incoming data until a tab is found.
        szParam[iSize] = c;
        iSize++;
      }
    }
  }


  Serial.println(F("timeout"));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool getConfigFromPC() {

  char szParam[45];
  unsigned long iMilliTimeout = millis() + 10000;    //wait up to 10 seconds for this data

  while (millis() < iMilliTimeout) {

    while (!Serial.available()) {
      //wait
    }

    if (Serial.read() == 0x01) {
      Serial.println(F("Reading..."));

      //we have the start to a config string


      readConfigParam(szParam, sizeof(szParam));    //should be PT0002
      if (strcmp(szParam, CONFIG_VERSION) != 0) {
        //not a config string
        Serial.println(F("Failed to find Config Type param."));
        Serial.print(F("Found: "));
        Serial.println(szParam);
        return false;
      }

      readConfigParam(szParam, sizeof(Config.Callsign));    //Callsign
      strcpy(Config.Callsign, szParam);
      readConfigParam(szParam, 1);    //Callsign SSID
      Config.CallsignSSID = szParam[0];

      readConfigParam(szParam, sizeof(Config.Destination));    //Destination
      strcpy(Config.Destination, szParam);
      readConfigParam(szParam, 1);    //SSID
      Config.DestinationSSID = szParam[0];

      readConfigParam(szParam, sizeof(Config.Path1));    //Path1
      strcpy(Config.Path1, szParam);
      readConfigParam(szParam, 1);    //SSID
      Config.Path1SSID = szParam[0];

      readConfigParam(szParam, sizeof(Config.Path2));    //Path2
      strcpy(Config.Path2, szParam);
      readConfigParam(szParam, 1);    //SSID
      Config.Path2SSID = szParam[0];

      //Cutoff altitude to stop using the path
      readConfigParam(szParam, sizeof(szParam));
      Config.DisablePathAboveAltitude = atoi(szParam);

      //Symbol/Page
      readConfigParam(szParam, 1);
      Config.Symbol = szParam[0];
      readConfigParam(szParam, 1);
      Config.SymbolPage = szParam[0];


      //BeaconType
      readConfigParam(szParam, sizeof(szParam));
      Config.BeaconType = atoi(szParam);

      //Simple Beacon Delay
      readConfigParam(szParam, sizeof(szParam));
      Config.BeaconSimpleDelay = atoi(szParam);

      //SpeedBeacon
      readConfigParam(szParam, sizeof(szParam));
      Config.BeaconSpeedThreshLow = atoi(szParam);

      readConfigParam(szParam, sizeof(szParam));
      Config.BeaconSpeedThreshHigh = atoi(szParam);

      readConfigParam(szParam, sizeof(szParam));
      Config.BeaconSpeedDelayLow = atoi(szParam);

      readConfigParam(szParam, sizeof(szParam));
      Config.BeaconSpeedDelayMid = atoi(szParam);

      readConfigParam(szParam, sizeof(szParam));
      Config.BeaconSpeedDelayHigh = atoi(szParam);

      //AltitudeBeacon
      readConfigParam(szParam, sizeof(szParam));
      Config.BeaconAltitudeThreshLow = atoi(szParam);

      readConfigParam(szParam, sizeof(szParam));
      Config.BeaconAltitudeThreshHigh = atoi(szParam);

      readConfigParam(szParam, sizeof(szParam));
      Config.BeaconAltitudeDelayLow  = atoi(szParam);

      readConfigParam(szParam, sizeof(szParam));
      Config.BeaconAltitudeDelayMid  = atoi(szParam);

      readConfigParam(szParam, sizeof(szParam));
      Config.BeaconAltitudeDelayHigh = atoi(szParam);

      //Time Slots
      readConfigParam(szParam, sizeof(szParam));
      Config.BeaconSlot1 = atoi(szParam);

      readConfigParam(szParam, sizeof(szParam));
      Config.BeaconSlot2 = atoi(szParam);


      //Status Message
      readConfigParam(szParam, sizeof(szParam));
      strcpy(Config.StatusMessage, szParam);


      //Misc Flags
      readConfigParam(szParam, sizeof(szParam));
      Config.StatusXmitGPSFix = szParam[0] == '1';

      readConfigParam(szParam, sizeof(szParam));
      Config.StatusXmitBurstAltitude = szParam[0] == '1';

      readConfigParam(szParam, sizeof(szParam));
      Config.StatusXmitBatteryVoltage = szParam[0] == '1';

      readConfigParam(szParam, sizeof(szParam));
      Config.StatusXmitTemp = szParam[0] == '1';

      readConfigParam(szParam, sizeof(szParam));
      Config.StatusXmitPressure = szParam[0] == '1';

      readConfigParam(szParam, sizeof(szParam));
      Config.GPSSerialBaud = atoi(szParam);    //1=300, 2=1200, 3=2400, 4=4800, 5=9600, 6=19200

      readConfigParam(szParam, sizeof(szParam));
      Config.GPSSerialInvert = atoi(szParam);    //Invert the incoming signal

      readConfigParam(szParam, sizeof(szParam));
      Config.GPSType = atoi(szParam);        //0=Generic NMEA, 1=Ublox

      //Annunciator Type
      readConfigParam(szParam, sizeof(szParam));
      Config.AnnounceMode = atoi(szParam);


      unsigned int iCheckSum = 0;
      for (int i=0; i<7; i++) {
        iCheckSum += Config.Callsign[i];
      }
      Config.CheckSum = iCheckSum;
      return true;    //done reading in the file
    }
  }
  return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void sendConfigToPC() {
//dump the configs out to the host PC
        Serial.write(0x01);
        Serial.write(CONFIG_VERSION);
        Serial.write(0x09);

        Serial.write(Config.Callsign);
        Serial.write(0x09);
        Serial.write(Config.CallsignSSID);
        Serial.write(0x09);
        Serial.write(Config.Destination);
        Serial.write(0x09);
        Serial.write(Config.DestinationSSID);
        Serial.write(0x09);
        Serial.write(Config.Path1);
        Serial.write(0x09);
        Serial.write(Config.Path1SSID);
        Serial.write(0x09);
        Serial.write(Config.Path2);
        Serial.write(0x09);
        Serial.write(Config.Path2SSID);
        Serial.write(0x09);

        //Allow to disable the path above certain altitude
        Serial.print(Config.DisablePathAboveAltitude, DEC);
        Serial.write(0x09);

        //Symbol
        Serial.write(Config.Symbol);
        Serial.write(0x09);
        Serial.write(Config.SymbolPage);
        Serial.write(0x09);

        //Beacon Type
        Serial.print(Config.BeaconType, DEC);
        Serial.write(0x09);

        //Beacon - Simple Delay
        Serial.print(Config.BeaconSimpleDelay, DEC);
        Serial.write(0x09);


        //Beacon - Speed Beaconing
        Serial.print(Config.BeaconSpeedThreshLow, DEC);
        Serial.write(0x09);
        Serial.print(Config.BeaconSpeedThreshHigh, DEC);
        Serial.write(0x09);
        Serial.print(Config.BeaconSpeedDelayLow, DEC);
        Serial.write(0x09);
        Serial.print(Config.BeaconSpeedDelayMid, DEC);
        Serial.write(0x09);
        Serial.print(Config.BeaconSpeedDelayHigh, DEC);
        Serial.write(0x09);

        //Beacon - Altitude Beaconing
        Serial.print(Config.BeaconAltitudeThreshLow, DEC);
        Serial.write(0x09);
        Serial.print(Config.BeaconAltitudeThreshHigh, DEC);
        Serial.write(0x09);
        Serial.print(Config.BeaconAltitudeDelayLow, DEC);
        Serial.write(0x09);
        Serial.print(Config.BeaconAltitudeDelayMid, DEC);
        Serial.write(0x09);
        Serial.print(Config.BeaconAltitudeDelayHigh, DEC);
        Serial.write(0x09);

        //Beacon - Time Slots
        Serial.print(Config.BeaconSlot1, DEC);
        Serial.write(0x09);
        Serial.print(Config.BeaconSlot2, DEC);
        Serial.write(0x09);

        //Status Message
        Serial.write(Config.StatusMessage);
        Serial.write(0x09);

        //Misc Flags
        if (Config.StatusXmitGPSFix) Serial.write("1");
        else Serial.write("0");
        Serial.write(0x09);

        if (Config.StatusXmitBurstAltitude) Serial.write("1");
        else Serial.write("0");
        Serial.write(0x09);

        if (Config.StatusXmitBatteryVoltage) Serial.write("1");
        else Serial.write("0");
        Serial.write(0x09);

        if (Config.StatusXmitTemp) Serial.write("1");
        else Serial.write("0");
        Serial.write(0x09);

        if (Config.StatusXmitPressure) Serial.write("1");
        else Serial.write("0");
        Serial.write(0x09);

        //GPS Serial Data
        Serial.print(Config.GPSSerialBaud, DEC);      //1=300, 2=1200, 3=2400, 4=4800, 5=9600, 6=19200
        Serial.write(0x09);

        if (Config.GPSSerialInvert) Serial.write("1");   //1=Invert the incoming signal
        else Serial.write("0");
        Serial.write(0x09);

        Serial.print(Config.GPSType, DEC);      //0=Generic NMEA, 1=Ublox
        Serial.write(0x09);

        //Misc System Configuration
        Serial.print(Config.AnnounceMode, DEC);    //0=No annunciator, 1=LED only, 2=LED and buzzer

        Serial.write(0x04);      //End of string
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
