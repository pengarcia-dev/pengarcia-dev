#include <Arduino.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define PIN_REF_VOLTAGE 34 
#define PIN_EXIT_VOLTAGE 33
#define PIN_DAC_VOLTAGE 25
#define PIN_SWITCH 21

#define DEFAULT_VREF   985  
#define NO_OF_SAMPLES   300   

uint32_t raw;
uint32_t raw2;
uint32_t voltage;
uint32_t voltage_2;
char mensaje;
int valorDAC = 0;

esp_adc_cal_characteristics_t adc_chars;

void setup(void) {
  Serial.begin(115200);
  delay(1000);  
  pinMode(PIN_SWITCH, OUTPUT);
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_12);  // GPIO34 = ADC1_CHANNEL_6
  adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12);

  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, DEFAULT_VREF, &adc_chars);
}

void loop(void) {

  while(Serial.available()) {
    uint64_t sum_voltage_ref = 0;
    uint64_t sum_voltage_out = 0;
    mensaje = Serial.read();
    if(mensaje == 'a'){

      for (int i = 0; i < NO_OF_SAMPLES; i++) {
        raw = adc1_get_raw(ADC1_CHANNEL_6);  
        voltage = esp_adc_cal_raw_to_voltage(raw, &adc_chars); 
        sum_voltage_ref += voltage;
        delay(10);
      }

      float promedio = sum_voltage_ref / (float)NO_OF_SAMPLES * 986.4 / 1011.11;
      Serial.printf("El voltaje de la referencia es: %f mV \n", promedio);
      int dac_value = round(promedio - 40);
      valorDAC = round((dac_value / 3300.0) * 255.0);

      dacWrite(PIN_DAC_VOLTAGE, valorDAC);

      float voltajeReal = (valorDAC / 255.0) * 3.3 * 1000;
      Serial.printf("El voltaje DAC: %f mV \n", voltajeReal);
    }

    if(mensaje == 'c') {
      digitalWrite(PIN_SWITCH, HIGH);
      for (int i = 0; i < NO_OF_SAMPLES; i++) {
        raw2 = adc1_get_raw(ADC1_CHANNEL_5);  
        voltage_2 = esp_adc_cal_raw_to_voltage(raw2, &adc_chars); 
        sum_voltage_out += voltage_2;
        delay(10);
      }
      float promedio2 = sum_voltage_out / (float)NO_OF_SAMPLES;
      Serial.println(promedio2);
    }
  }

 
}
