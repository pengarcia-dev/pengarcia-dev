#include <Arduino.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"

/*
  === DESCRIPTION ===
  This program performs three main functions via serial commands:
  - Command 'a': Measures a reference voltage on GPIO34 and sets the DAC (GPIO25) to a value 40 mV below that reference.
  - Command 'c': Activates a switch (GPIO21) and measures the output voltage on GPIO33.
  - Command 'r': Captures a time series of voltage measurements from GPIO34, useful for observing transient responses (e.g., capacitor charging).

  Target Platform: ESP32
*/

// === Pin configuration ===
#define PIN_REF_VOLTAGE 34   // GPIO34: analog input to measure reference voltage
#define PIN_EXIT_VOLTAGE 33  // GPIO33: analog input to measure output voltage
#define PIN_DAC_VOLTAGE 25   // GPIO25: DAC output pin
#define PIN_SWITCH 21        // GPIO21: digital output to activate an external load/switch

// === ADC calibration settings ===
#define DEFAULT_VREF 985     // Estimated internal reference voltage for ADC calibration (in mV)
#define NO_OF_SAMPLES 300    // Number of ADC samples for averaging

// === Global variables ===
uint32_t raw, raw2;          // Raw ADC readings
uint32_t voltage, voltage_2; // Calibrated voltages (mV)
char mensaje;                // Serial command input
int valorDAC = 0;            // 8-bit DAC output value (0–255)

// === ADC calibration structure ===
esp_adc_cal_characteristics_t adc_chars;

void setup(void) {
  Serial.begin(115200);
  delay(1000); // Wait for Serial Monitor to be ready

  pinMode(PIN_SWITCH, OUTPUT);

  // Configure ADC resolution and attenuation
  adc1_config_width(ADC_WIDTH_BIT_12); // 12-bit resolution (0–4095)
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_12); // GPIO34
  adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12); // GPIO33

  // Calibrate ADC with default reference voltage
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
    // Variables for averaging
    uint64_t sum_voltage_ref = 0;
    uint64_t sum_voltage_out = 0;
    float average_ref = 0;
    float average_out = 0;
    float actual_dac_voltage = 0;
    int dac_target_mV = 0;

    mensaje = Serial.read(); // Read incoming command

    // === Command 'a': Set DAC output to (reference voltage - 40 mV) ===
    if (mensaje == 'a') {
      for (int i = 0; i < NO_OF_SAMPLES; i++) {
        raw = adc1_get_raw(ADC1_CHANNEL_6); // Read GPIO34
        voltage = esp_adc_cal_raw_to_voltage(raw, &adc_chars); // Convert to mV
        sum_voltage_ref += voltage;
        delay(10);
      }

      // Compute average and apply correction factor
      average_ref = sum_voltage_ref / (float)NO_OF_SAMPLES * 986.4 / 1011.11;

      Serial.printf("Reference voltage: %.2f mV\n", average_ref);

      // Calculate DAC value: target = ref - 40 mV
      dac_target_mV = round(average_ref - 40);
      valorDAC = round((dac_target_mV / 3300.0) * 255.0);
      dacWrite(PIN_DAC_VOLTAGE, valorDAC);

      // Compute actual DAC voltage based on 8-bit range
      actual_dac_voltage = (valorDAC / 255.0) * 3.3 * 1000;
      Serial.printf("DAC output voltage: %.2f mV\n", actual_dac_voltage);
    }

    // === Command 'c': Activate switch and measure output voltage ===
    if (mensaje == 'c') {
      digitalWrite(PIN_SWITCH, HIGH); // Activate output/load

      for (int i = 0; i < NO_OF_SAMPLES; i++) {
        raw2 = adc1_get_raw(ADC1_CHANNEL_5); // Read GPIO33
        voltage_2 = esp_adc_cal_raw_to_voltage(raw2, &adc_chars); // Convert to mV
        sum_voltage_out += voltage_2;
        delay(10);
      }

      average_out = sum_voltage_out / (float)NO_OF_SAMPLES;
      Serial.printf("Measured output voltage (GPIO33): %.2f mV\n", average_out);
    }

    // === Command 'r': Capture voltage variation over time on GPIO34 ===
    if (mensaje == 'r') {
      digitalWrite(PIN_SWITCH, LOW); // Ensure load is off

      const int N = 200;     // Number of samples
      float voltajes[N];     // Array to store sampled voltages

      for (int i = 0; i < N; i++) {
        uint64_t sum_voltage = 0;

        for (int j = 0; j < 10; j++) {
          raw = adc1_get_raw(ADC1_CHANNEL_6);
          voltage = esp_adc_cal_raw_to_voltage(raw, &adc_chars);
          sum_voltage += voltage;
          delay(1); // Sampling delay
        }

        // Average and apply two-stage empirical correction
        float average = sum_voltage / 10.0 * 986.4 / 1011.11 * 988.4 / 992.05;
        voltajes[i] = average;

        delay(3); // Control total sampling rate (~200 Hz)
      }

      // Print the captured voltage data
      Serial.println("Measured voltages:");
      for (int i = 0; i < N; i++) {
        Serial.printf("%d: %.2f mV\n", i, voltajes[i]);
      }
    }
  }
}
