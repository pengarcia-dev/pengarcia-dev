#include <Arduino.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"

// === Pin definitions ===
#define PIN_REF_VOLTAGE 34     // GPIO34: analog input to measure reference voltage
#define PIN_EXIT_VOLTAGE 33    // GPIO33: analog input to measure output voltage
#define PIN_DAC_VOLTAGE 25     // GPIO25: DAC output pin
#define PIN_SWITCH 21          // GPIO21: digital output to control external switch (or load)

// === ADC calibration and sample settings ===
#define DEFAULT_VREF    985    // Estimated reference voltage in mV (adjust according to your board)
#define NO_OF_SAMPLES   300    // Number of ADC samples to average

// === Variables ===
uint32_t raw;           // Raw ADC reading for GPIO34
uint32_t raw2;          // Raw ADC reading for GPIO33
uint32_t voltage;       // Calibrated voltage in mV (for GPIO34)
uint32_t voltage_2;     // Calibrated voltage in mV (for GPIO33)
char mensaje;           // Serial input command
int valorDAC = 0;       // DAC output value (0–255)

// === ADC calibration structure ===
esp_adc_cal_characteristics_t adc_chars;

void setup(void) {
  Serial.begin(115200);
  delay(1000);  // Wait for serial monitor to open

  pinMode(PIN_SWITCH, OUTPUT);  // Set switch control pin as OUTPUT

  // === Configure ADC width and attenuation ===
  adc1_config_width(ADC_WIDTH_BIT_12);  // 12-bit ADC resolution (0–4095)
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_12);  // GPIO34
  adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12);  // GPIO33

  // === Calibrate ADC with default reference voltage ===
  esp_adc_cal_characterize(
    ADC_UNIT_1,
    ADC_ATTEN_DB_12,
    ADC_WIDTH_BIT_12,
    DEFAULT_VREF,
    &adc_chars
  );
}

void loop(void) {
  while (Serial.available()) {
    // Accumulators for averaging
    uint64_t sum_voltage_ref = 0;
    uint64_t sum_voltage_out = 0;

    mensaje = Serial.read();  // Read incoming serial character

    // === Handle 'a': Measure reference voltage and set DAC output ===
    if (mensaje == 'a') {
      for (int i = 0; i < NO_OF_SAMPLES; i++) {
        raw = adc1_get_raw(ADC1_CHANNEL_6);  // Read raw ADC value from GPIO34
        voltage = esp_adc_cal_raw_to_voltage(raw, &adc_chars);  // Convert to mV
        sum_voltage_ref += voltage;
        delay(10);
      }

      // Apply scaling factor based on calibration (adjust if needed)
      float average_ref = sum_voltage_ref / (float)NO_OF_SAMPLES * 986.4 / 1011.11;
      Serial.printf("Reference voltage: %.2f mV\n", average_ref);

      // Calculate DAC value to set based on measured reference
      int dac_target_mV = round(average_ref - 40);  // Subtract offset if needed
      valorDAC = round((dac_target_mV / 3300.0) * 255.0);  // Convert to 8-bit DAC range

      dacWrite(PIN_DAC_VOLTAGE, valorDAC);  // Output DAC signal on GPIO25

      float actual_dac_voltage = (valorDAC / 255.0) * 3.3 * 1000;  // Convert back to mV
      Serial.printf("DAC output voltage: %.2f mV\n", actual_dac_voltage);
    }

    // === Handle 'c': Activate switch and measure output voltage on GPIO33 ===
    if (mensaje == 'c') {
      digitalWrite(PIN_SWITCH, HIGH);  // Activate switch/output
      for (int i = 0; i < NO_OF_SAMPLES; i++) {
        raw2 = adc1_get_raw(ADC1_CHANNEL_5);  // Read raw ADC value from GPIO33
        voltage_2 = esp_adc_cal_raw_to_voltage(raw2, &adc_chars);  // Convert to mV
        sum_voltage_out += voltage_2;
        delay(10);
      }

      float average_out = sum_voltage_out / (float)NO_OF_SAMPLES;
      Serial.printf("Measured output voltage (GPIO33): %.2f mV\n", average_out);
    }
    if (mensaje == 'r') {
      digitalWrite(PIN_SWITCH, HIGH);  // Activate switch/output
      for (int i = 0; i < 60; i++) {
        float average_out = 0;
        sum_voltage_out = 0;
        for (int i = 0; i < 50; i++) {
          raw2 = adc1_get_raw(ADC1_CHANNEL_5);  // Read raw ADC value from GPIO33
          voltage_2 = esp_adc_cal_raw_to_voltage(raw2, &adc_chars);  // Convert to mV
          sum_voltage_out += voltage_2;
          delay(10);
        }

        average_out = sum_voltage_out / (float)NO_OF_SAMPLES;
        Serial.printf("Measured output voltage (GPIO33): %.2f mV\n", average_out);
        delay(10);
      }



      

    }
  }
}
