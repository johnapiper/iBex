void Stage1 () {
  if(Time_Stage1==0) {
    debug = "STAGE 1";
  }
Time_Stage1 = millis();
    if(SystemCheck_Pass == 1) {
    Stage++;
    return;
    }
return;
}

void Stage2 () {
Time_Stage2 = millis();
    if(Alt_Ascent >= Alt_Stage3Trigger) {
    Stage++;
    return;
    }
return;
}

void Stage3 () {
Time_Stage3 = millis();
    if(Alt_Ascent >= Alt_Stage4Trigger) {
    Stage++;
    return;
    }
return;
}

void Stage4 () {
Time_Stage4 = millis();
    if(Alt_Ascent >= Alt_Stage5Trigger) {
    Stage++;
    return;
    }
return;
}

void Stage5 () {
Time_Stage5 = millis();
return;
}

void Stage6 () {
Time_Stage6 = millis();
return;
}

void Stage7 () {
Time_Stage7 = millis();
return;
}
