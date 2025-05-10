#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println("Introduce un valor en milivoltios (0 - 3300):");
}

void loop() {
  if (Serial.available()) {
    int mV = Serial.parseInt();  // Leemos en milivoltios
    if (mV >= 0 && mV <= 3300) {
      int valorDAC = round((mV / 3300.0) * 255.0);  // Conversión precisa
      dacWrite(25, valorDAC);
      
      float voltajeReal = (valorDAC / 255.0) * 3.3;  // Estimación de salida en voltios
      
      Serial.print("Entrada: ");
      Serial.print(mV);
      Serial.print(" mV | DAC = ");
      Serial.print(valorDAC);
      Serial.print(" → Voltaje estimado: ");
      Serial.print(voltajeReal, 2);
      Serial.println(" V");
    } else {
      Serial.println("Fuera de rango (0 - 3300 mV)");
    }
  }
}
