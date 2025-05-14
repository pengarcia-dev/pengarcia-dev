#include <Arduino.h>

#define PIN_DAC DAC1
#define PIN_SWITCH 21
#define PIN_EXIT_VOLTAGE 33
#define PIN_REF_VOLTAGE 34

int ref_v_value = 0;
float v_value = 0.0;
float media = 0.0;
float v_value_prom = 0.0;

int i = 0;

void setup(void) {
  Serial.begin(115200);
  ref_v_value = analogRead(PIN_REF_VOLTAGE);
  v_value = map(ref_v_value, 0, 4096, 0, 3300);

  for(i; i < 300; i++) {
    ref_v_value = analogRead(PIN_REF_VOLTAGE);
    v_value = map(ref_v_value, 0, 4096, 0, 3300);
    Serial.printf("valor %d de voltaje medido: %f mV \n", i, v_value);
    media = media + v_value;
    delay(10);

  }
  
  v_value_prom = media / i;

  Serial.println(v_value_prom); 

}

void loop(void) {


}