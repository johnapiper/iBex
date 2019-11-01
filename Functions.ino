void Sensor_Read() {
  sensor.ReadProm();
  sensor.Readout();
  MS5607_Temp=sensor.GetTemp();
  MS5607_Pressure=sensor.GetPres();
  Alt_Actual=((1-(pow((MS5607_Pressure/Press_Sea),0.190284)))*145366.45);
  Alt_Ascent=((1-(pow((MS5607_Pressure/Press_Release),0.190284)))*145366.45);

  Time_SensorRead=Time;
return;
}

void Debug_ALLSensors() {
  
  debug="  -  MS5607 Temperature Reading:  ";
  debug2=MS5607_Temp;
  Debug();
  Debug2();
  DebugTerm();
  debug="  -  MS5607 Pressure Reading:  ";
  debug2=MS5607_Pressure;
  Debug();
  Debug2();
  DebugTerm();
  debug="  -  Altimeter:  ";
  debug2=Alt_Actual;
  Debug();
  Debug2();
  DebugTerm();
  debug="  -  Ascent:  ";
  debug2=Alt_Ascent;
  Debug();
  Debug2();
  DebugTerm();

  Time_SensorDebug=Time;
return;
}

void Debug() {
if (debug_Last == debug) {return;}
Serial.print("Debug - ");
Serial.print(Time_Stamp);
Serial.print(": ");
Serial.print(debug);
debug_Last = debug;
return;
}
void Debug2() {
Serial.print(debug2);
return;
}
void Debug3() {
return;  
}
void DebugTerm() {
Serial.println("");
return;
}

void Jettison() {
return;
}

void TimeHandler() {
Time=millis();
long allSeconds=Time/1000;
int runHours= allSeconds/3600;
int secsRemaining=allSeconds%3600;
int runMinutes=secsRemaining/60;
int runSeconds=secsRemaining%60;

sprintf(Time_Stamp,"T+%02dh:%02dm:%02ds",runHours,runMinutes,runSeconds);
return;
}

void ErrorHandler () {
  debug = "========== >>> Error Report <<< ==========";
  Debug();
  DebugTerm();
  debug = "Ballon Stage: ";
  debug2=Stage;
  Debug();
  Debug2();
  DebugTerm();
  debug = "";
  Debug();
  DebugTerm();
  debug = "Error code: ";
  debug2=Failure_ID;
  Debug();
  Debug2();
  DebugTerm();
  debug=Failure_Reason;
  Debug();
  DebugTerm();
  debug="Time of Failure: ";
  Debug();
  DebugTerm();
  debug=Time_Failure;
  Debug();
  DebugTerm();
  debug = "========== >>> End of Report <<< ==========";
  Debug();
  DebugTerm();
  debug = "";
  Debug();
  DebugTerm();
return;
}

void ConcatData() {
sprintf(DataByte, "a",Lat1,"b",Long1,"c",signalQuality,"d",MS5607_Temp);
return;
}

