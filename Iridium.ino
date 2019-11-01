void Iridium() {
  ConcatData();
  int err;
  
  DebugTerm();
  debug="=============  IRIDIUM RECIEVE  ====================";
  DebugTerm();
  Debug();


//Receive Data

  
  // Read/Write the first time or if there are any remaining messages
  if (!messageSent || modem.getWaitingMessageCount() > 0)
  {
    size_t bufferSize = sizeof(buffer);

    // First time through send+receive; subsequent loops receive only
    if (!messageSent)
      err = modem.sendReceiveSBDBinary(buffer, 11, buffer, bufferSize);
    else
      err = modem.sendReceiveSBDText(NULL, buffer, bufferSize);
      
    if (err != ISBD_SUCCESS)
    {
      Serial.print("sendReceiveSBD* failed: error ");
      Serial.println(err);
    }
    else // success!
    {
      messageSent = true;
      Serial.print("Inbound buffer size is ");
      Serial.println(bufferSize);
      for (int i=0; i<bufferSize; ++i)
      {
        Serial.print(buffer[i], HEX);
        if (isprint(buffer[i]))
        {
          Serial.print("(");
          Serial.write(buffer[i]);
          Serial.print(")");
        }
        Serial.print(" ");
      }
      Serial.println();
      Serial.print("Messages remaining to be retrieved: ");
      Serial.println(modem.getWaitingMessageCount());
    }
  }

  
  DebugTerm();
  debug="=============  IRIDIUM END OF RECEIVE  ====================";
  DebugTerm();
  Debug();


  
  DebugTerm();
  debug="=============  IRIDIUM SEND  ====================";
  DebugTerm();
  Debug();

  
//Send Data

  err = modem.sendSBDText((DataByte));
  if (err != ISBD_SUCCESS)
  {
    Serial.print("sendSBDText failed: error ");
    Serial.println(err);
    if (err == ISBD_SENDRECEIVE_TIMEOUT)
      Serial.println("Try again with a better view of the sky.");
  }
  else
  {
    Serial.println("Data Sent");
  }
Time_Iridium=Time;
  
  DebugTerm();
  debug="=============  IRIDIUM END OF SEND  ====================";
  DebugTerm();
  Debug();

return;
}



void Iridium_SignalCheck() {

  
  DebugTerm();
  debug="=============  IRIDIUM Signal Checker  ====================";
  DebugTerm();
  Debug();

  
    err = modem.getSignalQuality(signalQuality);
  if (err != 0)
  {
    debug=("Iridium: SignalQuality failed:        Iridium_Error Status =  ");
    debug2=(signalQuality*1);
    Debug();
    Debug2();
    DebugTerm();
    debug=("");
    Debug();
    DebugTerm();
    return;
  }
  else
  debug=("Iridium Module Connected:        Quality =  ");
  debug2=(signalQuality*1);
  Debug();
  Debug2();
  DebugTerm();
  debug=("");
  Debug();
  DebugTerm();

Time_IridiumSignalCheck=Time;

  DebugTerm();
  debug="=============  IRIDIUM Signal Checker COMPLETE  ====================";
  DebugTerm();
  Debug();

return;
}







#if DIAGNOSTICS
void ISBDConsoleCallback(IridiumSBD *device, char c)
{
  Serial.write(c);
}

void ISBDDiagsCallback(IridiumSBD *device, char c)
{
  Serial.write(c);
}
#endif
