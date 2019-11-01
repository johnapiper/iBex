/*
Ibex

*/

#include <Wire.h>
#include <MS5xxx.h>

MS5xxx sensor(&Wire);
double temp=0;
void setup() {
  Serial.begin(9600);
  if(sensor.connect()>0) {
    Serial.println("Error connecting...");
    delay(500);
    setup();
  }
}

void loop() {
  sensor.ReadProm();
  sensor.Readout();
  Serial.print("Temperature [0.01 C]: ");
  temp=sensor.GetTemp();
  Serial.println(temp);
  Serial.print("Pressure [Pa]: ");
  Serial.println(sensor.GetPres());
  Serial.println("---");
  delay(500);
}

