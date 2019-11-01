void System_Check() {
  debug="Running System Diagnostic";
  Debug();
  DebugTerm();
  Sensor_Read();

  //Failure Checks
  if (Press_Release==0){
      Time_Failure = Time_Stamp;
      Failure_ID = 101;
      Failure_Reason = "MS5607 - Atmosphere Pressure Calculation for release location FAILED";
      ErrorHandler();
 return;
  }
  SystemCheck_Pass = 1;
return;
}
