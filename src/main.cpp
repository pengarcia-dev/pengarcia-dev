#include <Arduino.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define PIN_REF_VOLTAGE 34  // GPIO34 conectado al voltaje que quieres medir

// Parámetros de calibración
#define DEFAULT_VREF   985  // Valor típico del Vref en mV. Puedes ajustarlo si lo conoces mejor.
#define NO_OF_SAMPLES   300   // Número de muestras para promediar

// Características del ADC calibrado
esp_adc_cal_characteristics_t adc_chars;

void setup(void) {
  Serial.begin(115200);
  delay(1000);  // Espera inicial para estabilizar

  // Configura ADC1 en 12 bits y con atenuación 11dB (para voltajes hasta ~3.3V)
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_12);  // GPIO34 = ADC1_CHANNEL_6

  // Caracteriza el ADC con la referencia por defecto
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, DEFAULT_VREF, &adc_chars);

  uint32_t raw;
  uint32_t voltage;
  uint64_t sum_voltage = 0;

  for (int i = 0; i < NO_OF_SAMPLES; i++) {
    raw = adc1_get_raw(ADC1_CHANNEL_6);  // Lectura cruda del ADC
    voltage = esp_adc_cal_raw_to_voltage(raw, &adc_chars);  // Conversión calibrada a mV
    Serial.printf("valor %d de voltaje medido: %d mV\n", i, voltage);
    sum_voltage += voltage;
    delay(10);
  }

  float promedio = sum_voltage / (float)NO_OF_SAMPLES;
  Serial.printf("\n✅ Voltaje promedio corregido: %.2f mV\n", promedio);
}

void loop(void) {
  // No se hace nada en el bucle principal
}
