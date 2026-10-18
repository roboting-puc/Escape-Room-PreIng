/*
  ==========================================================
   ESCALERA AUTOMATICA  (Nivel 2)
   Los LEDs se van encendiendo uno tras otro, como las luces
   de una escalera cuando alguien sube y luego baja.
  ==========================================================
   Conexiones (segun el plano, de izquierda a derecha):
     LED 1 -> pin 4
     LED 2 -> pin 3
     LED 3 -> pin 2
     (cada LED con su resistor a GND)
  ==========================================================
*/

const int LEDS[] = {4, 3, 2};   // de izquierda a derecha
const int N = 3;

const int PASO_MS  = 350;   // tiempo entre escalon y escalon
const int PAUSA_MS = 700;

void apagarTodo() {
  for (int i = 0; i < N; i++) digitalWrite(LEDS[i], LOW);
}

void setup() {
  for (int i = 0; i < N; i++) pinMode(LEDS[i], OUTPUT);
}

void loop() {
  // 1) Alguien SUBE: se encienden de a uno
  for (int i = 0; i < N; i++) { digitalWrite(LEDS[i], HIGH); delay(PASO_MS); }
  delay(PAUSA_MS);
  // ...y se van apagando en el mismo orden
  for (int i = 0; i < N; i++) { digitalWrite(LEDS[i], LOW);  delay(PASO_MS); }
  delay(PAUSA_MS);

  // 2) Alguien BAJA: al reves
  for (int i = N - 1; i >= 0; i--) { digitalWrite(LEDS[i], HIGH); delay(PASO_MS); }
  delay(PAUSA_MS);
  for (int i = N - 1; i >= 0; i--) { digitalWrite(LEDS[i], LOW);  delay(PASO_MS); }
  delay(PAUSA_MS);

  // 3) Alguien sube corriendo (3 veces rapido)
  for (int vuelta = 0; vuelta < 3; vuelta++) {
    for (int i = 0; i < N; i++) {
      digitalWrite(LEDS[i], HIGH);
      delay(110);
      digitalWrite(LEDS[i], LOW);
    }
  }
  apagarTodo();
  delay(PAUSA_MS);
}
