/*
  ==========================================================
   ESPANTACUCO  (Nivel 1)
   El potenciometro regula la intensidad de la luz del LED.
  ==========================================================
   Conexiones (segun el plano):
     Potenciometro: pata del medio -> A5
                    patas de los lados -> 5V y GND
     LED (con resistor) -> pin 3 (pin PWM "~")
  ==========================================================
*/

const int PIN_POT = A5;
const int PIN_LED = 3;   // tiene que ser PWM (~): 3, 5, 6, 9, 10 u 11

void setup() {
  pinMode(PIN_LED, OUTPUT);
}

void loop() {
  int lectura = analogRead(PIN_POT);   // 0 .. 1023

  // Curva cuadratica: el ojo no ve el brillo de forma lineal,
  // asi la perilla se siente pareja en todo su recorrido.
  long brillo = (long)lectura * lectura / 4100;   // 0 .. 255
  if (brillo > 255) brillo = 255;

  analogWrite(PIN_LED, (int)brillo);
  delay(10);
}
