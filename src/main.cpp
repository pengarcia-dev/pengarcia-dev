#include <Arduino.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"

// === Pin definitions ===
#define PIN_REF_VOLTAGE 34     // GPIO34: analog input for reference voltage
#define PIN_OUTPUT_VOLTAGE 33  // GPIO33: analog input for DAC-controlled output
#define PIN_DAC_OUTPUT 25      // GPIO25: DAC output pin
#define PIN_SWITCH 21          // GPIO21: optional digital control (e.g. enable relay)

// === Calibration and control parameters ===
#define DEFAULT_VREF    985      // Internal reference voltage in mV (adjust if needed)
#define NO_OF_SAMPLES   50       // ADC samples for averaging
#define GAIN            0.2      // Proportional gain for control loop

// === Global variables ===
esp_adc_cal_characteristics_t adc_chars;
uint32_t raw_ref, raw_out;
float voltage_ref, voltage_out;
int dac_value = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_SWITCH, OUTPUT);
  digitalWrite(PIN_SWITCH, HIGH);  // Enable switch/load

  // === Configure ADC channels ===
  adc1_config_width(ADC_WIDTH_BIT_12);  // 12-bit ADC resolution
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_12);  // GPIO34
  adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12);  // GPIO33

  // === Calibrate ADC ===
  esp_adc_cal_characterize(
    ADC_UNIT_1,
    ADC_ATTEN_DB_12,
    ADC_WIDTH_BIT_12,
    DEFAULT_VREF,
    &adc_chars
  );

  Serial.println("Closed-loop DAC control initialized.");
}

void loop() {
  float sum_ref = 0;
  float sum_out = 0;

  // === Average ADC readings ===
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
    raw_ref = adc1_get_raw(ADC1_CHANNEL_6);  // Reference voltage
    raw_out = adc1_get_raw(ADC1_CHANNEL_5);  // Output voltage
    sum_ref += esp_adc_cal_raw_to_voltage(raw_ref, &adc_chars);
    sum_out += esp_adc_cal_raw_to_voltage(raw_out, &adc_chars);
    delay(5);
  }

  voltage_ref = sum_ref / NO_OF_SAMPLES;
  voltage_out = sum_out / NO_OF_SAMPLES;

  // === Print current status ===
  Serial.printf("Reference voltage: %.2f mV\n", voltage_ref);
  Serial.printf("Output voltage:    %.2f mV\n", voltage_out);

  // === Compute error and adjust DAC output ===
  float error = voltage_ref - voltage_out;
  float correction = GAIN * error;
  dac_value += round((correction / 3300.0) * 255.0);  // Scale to 8-bit DAC

  // === Clamp DAC value between 0–255 ===
  dac_value = constrain(dac_value, 0, 255);
  dacWrite(PIN_DAC_OUTPUT, dac_value);

  float actual_dac_mv = (dac_value / 255.0) * 3.3 * 1000;
  Serial.printf("DAC output voltage: %.2f mV (DAC value = %d)\n\n", actual_dac_mv, dac_value);

  delay(300);  // Loop delay
}
