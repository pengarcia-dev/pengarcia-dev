#include <Arduino.h>

#define PIN_DAC DAC1
#define PIN_INTERRUPT 21


void setup(){
  Serial.begin(115200);
  Serial.println("Simple operation of Potentiometer with DAC");
  delay(2000);}

void loop(){
  if(Serial.available()>0){
    int mV = Serial.parseInt();
    if(mV >= 0 && mV <= 3300){
      Serial.printf("V_DAC = %d mV\n", mV);
      dacWrite(PIN_DAC, mV);
      delay(1000);
    }
  }
}
