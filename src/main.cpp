#include <Arduino.h>

#define PIN_DAC DAC1
#define PIN_SWITCH 21
#define PIN_EXIT_VOLTAGE 33
#define PIN_REF_VOLTAGE 34

uint8_t ref_v_value = 0;

void setup(void) {
  Serial.begin(115200);
  ref_v_value = analogRead(PIN_REF_VOLTAGE);
  Serial.println(ref_v_value);


  

}

void loop(void) {


}