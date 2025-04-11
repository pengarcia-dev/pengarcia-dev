#include <Arduino.h>

bool dac_set = false; // Variable para controlar si el DAC ya ha sido configurado
float contraelectrodo = .0;
float dac_value = 0;
float valor_pila = 0;
#define SWITCH_PIN 17
#define ADC_POST_SWITCH A7

float leerVoltajePromediado(int pin, int numLecturas)
{
  float suma = 0;
  for (int i = 0; i < numLecturas; i++)
  {
    suma += analogReadMilliVolts(pin) / 1000.0; // Leer en mV y convertir a V
    delay(10);                                  // Pequeño retraso para evitar lecturas seguidas con ruido
  }
  return suma / numLecturas;
}

void setup()
{
  Serial.begin(115200);
  pinMode(SWITCH_PIN, OUTPUT);
  // Configurar resolución del ADC a 12 bits (0-4095)
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db); // Permite medir hasta 3.3V sin recortes

  // Preguntar si se ha desconectado el interruptor
  Serial.println("Potenciostato");
  Serial.println("Escribe abrir o cerrar el interruptor");
}

void loop()
{
  while (!Serial.available())

    ; // Esperar entrada del usuario

  char respuesta = Serial.read(); // Leer la respuesta

  if (respuesta == 'a')
  {
    digitalWrite(SWITCH_PIN, LOW);
    Serial.println("Interruptor abierto");
    float valor_pila = leerVoltajePromediado(15, 150) * 0.97 / 1.01;

    Serial.println("Valor de la pila: " + String(valor_pila, 2) + " V (" + String(valor_pila * 1000, 2) + " mV)");

    dac_value = (valor_pila - 0.04) * 1000 * 255 / 3300;
    Serial.println(float(dac_value));
    dacWrite(DAC1, dac_value);
  }
  else if (respuesta == 'c')
  {
    digitalWrite(SWITCH_PIN, HIGH);
    Serial.println("DAC fijado");
    Serial.println("Interruptor cerrado");
    float valor_salida = leerVoltajePromediado(4, 150);
    Serial.println("La salida es " + String(valor_salida, 2) + " V (" + String(valor_salida * 1000, 2) + " mV)");
  }
  else
  {
    Serial.println("Error: Respuesta no válida");
  }

}
