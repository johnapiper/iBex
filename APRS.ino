void APRS_Transmit() {
  char szTemp[30];
  int i;
  double insideTemp;    //inside air temp
  double outsideTemp;    //outside air temp
  double airPressure;    //millibars
  float fTemp;    //temporary variable

  char statusIAT = 0;
  char statusOAT = 0;

  if (Config.StatusXmitPressure || Config.StatusXmitTemp) {
    //we're supposed to transmit the air pressure and/or temp - go ahead and pre-fetch it

    //First we need to capture the air temp (part of the pressure equation)
    char statusIAT = Pressure.startTemperature();
    if (statusIAT != 0) {
      // Wait for the measurement to complete:
      delay(statusIAT);

      statusIAT = Pressure.getTemperature(insideTemp);
      if (statusIAT != 0) {
        statusIAT = Pressure.startPressure(3);

        if (statusIAT != 0)
        {
          // Wait for the measurement to complete:
          delay(statusIAT);

          statusIAT = Pressure.getPressure(airPressure, insideTemp);
          if (statusIAT == 0)
          {
            //we had some sort of problem with getting the air pressure - set it to zero
            airPressure = 0.0;
          }
        }
      } else {
        //problem getting air temp
        insideTemp = 0.0;
        airPressure = 0.0;
      }
    }
    statusOAT = OAT.getTemperature(outsideTemp);
  }
  
  oTNC.xmitStart(Config.Destination, Config.DestinationSSID, Config.Callsign, Config.CallsignSSID, Config.Path1, Config.Path1SSID, Config.Path2, Config.Path2SSID, (GPSParser.Altitude() < Config.DisablePathAboveAltitude));


  //      /155146h3842.00N/09655.55WO301/017/A=058239
  int hh = 0, mm = 0, ss = 0;
  GPSParser.getGPSTime(&hh, &mm, &ss);
  oTNC.xmitString("/");

  sprintf(szTemp, "%02d", hh);
  oTNC.xmitString(szTemp);
  sprintf(szTemp, "%02d", mm);
  oTNC.xmitString(szTemp);
  sprintf(szTemp, "%02d", ss);
  oTNC.xmitString(szTemp);

  oTNC.xmitString("h");

  //Latitude
  GPSParser.getLatitude(szTemp);
  i=0;
  while (i<7 && szTemp[i]) {
    oTNC.xmitChar(szTemp[i]);
    i++;
  }
  oTNC.xmitChar(GPSParser.LatitudeHemi());

  oTNC.xmitChar(Config.SymbolPage);

  //Longitude
  GPSParser.getLongitude(szTemp);
  i=0;
  while (i<8 && szTemp[i]) {
    oTNC.xmitChar(szTemp[i]);
    i++;
  }
  oTNC.xmitChar(GPSParser.LongitudeHemi());

  oTNC.xmitChar(Config.Symbol);

  //Course
  fTemp = GPSParser.Course();

  sprintf(szTemp, "%03d", (int)fTemp);
  oTNC.xmitString(szTemp);

  oTNC.xmitChar('/');

  //Speed in knots
  fTemp = GPSParser.Knots();

  sprintf(szTemp, "%03d", (int)fTemp);
  oTNC.xmitString(szTemp);


  oTNC.xmitString("/A=");

  //Altitude in Feet
  fTemp = GPSParser.AltitudeInFeet();
  oTNC.xmitLong((long)fTemp, true);


  if (Config.StatusXmitGPSFix) {
    //Fix quality and num sats

    if (GPSParser.FixQuality() >= 1 && GPSParser.FixQuality() <=3) {
      //we have a GPS, DGPS, or PPS fix
      oTNC.xmitString(" 3D");
    } else {
      oTNC.xmitString(" na");
    }
    sprintf(szTemp, "%dSats", GPSParser.NumSats());
    oTNC.xmitString(szTemp);

  }

  if (Config.StatusXmitBatteryVoltage) {
    int iBattery = analogRead(PIN_ANALOG_BATTERY);
    float fVolts = (float)iBattery / 204.8;    //204.8 points per volt,
    fVolts = fVolts * 3.141;        //times (147/100) to adjust for the resistor divider
    fVolts = fVolts + 0.19;      //account for the inline diode on the power supply


    oTNC.xmitString(" Batt=");
    oTNC.xmitFloat(fVolts);

  }

  if (Config.StatusXmitTemp) {
    if (statusIAT) {
      oTNC.xmitString(" IAT=");
      oTNC.xmitFloat((float)insideTemp);
    }
    
    if (statusOAT) {
      oTNC.xmitString(" OAT=");
      oTNC.xmitFloat((float)outsideTemp);
    }
  }
  if (Config.StatusXmitPressure) {
    if (statusIAT) {
      oTNC.xmitString(" Press=");
      oTNC.xmitFloat((float)airPressure);
    }
  }


  if (Config.StatusXmitBurstAltitude && bHasBurst) {
    oTNC.xmitString(" Burst=");
    fTemp = fMaxAlt * METERS_TO_FEET;
    oTNC.xmitLong((long)fTemp, true);
  }

  oTNC.xmitChar(' ');
  oTNC.xmitString(Config.StatusMessage);

  oTNC.xmitEnd();
}
