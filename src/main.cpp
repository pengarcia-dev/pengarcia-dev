#include <Arduino.h>

#define PIN_DAC DAC1
#define PIN_SWITCH 21

void setup() {
  Serial.begin(115200);
  pinMode(PIN_SWITCH, OUTPUT);
  Serial.println("Introduce un valor en mV (0-3300), opcionalmente seguido de 'c' o 'a'.");
  Serial.println("Ejemplos: 1500 → ajusta DAC | a → abre switch | 1200b → ajusta DAC y cierra switch");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.length() == 0) return;

    // Variables
    int mV = -1;
    bool hayDAC = false;
    bool haySwitch = false;
    char comando = '\0';

    // Extraer número si lo hay
    int i = 0;
    while (i < input.length() && isDigit(input.charAt(i))) i++;

    if (i > 0) {
      mV = input.substring(0, i).toInt();
      hayDAC = true;
    }

    // Extraer comando si lo hay
    if (i < input.length()) {
      comando = input.charAt(i);
      if (comando == 'c' || comando == 'a') {
        haySwitch = true;
      }
    }

    // Ejecutar DAC
    if (hayDAC) {
      if (mV >= 0 && mV <= 3300) {
        int valorDAC = round((mV / 3300.0) * 255.0);
        dacWrite(PIN_DAC, valorDAC);

        float voltajeReal = (valorDAC / 255.0) * 3.3;
        Serial.print("DAC ajustado a ");
        Serial.print(mV);
        Serial.print(" mV → Valor DAC = ");
        Serial.print(valorDAC);
        Serial.print(" → Voltaje estimado: ");
        Serial.print(voltajeReal, 2);
        Serial.println(" V");
      } else {
        Serial.println("Valor DAC fuera de rango (0 - 3300 mV)");
      }
    }

    // Ejecutar comando de switch
    if (haySwitch) {
      if (comando == 'c') {
        Serial.println("Switch cerrado");
        digitalWrite(PIN_SWITCH, HIGH);
      } else if (comando == 'a') {
        Serial.println("Switch abierto");
        digitalWrite(PIN_SWITCH, LOW);
      }
    }
  }
}
