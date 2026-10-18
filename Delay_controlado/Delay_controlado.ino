/*
  ==========================================================
   DELAY CONTROLADO  (Nivel 1)
   El potenciometro regula la velocidad de parpadeo del LED.
  ==========================================================
   Conexiones (segun el plano):
     Potenciometro: pata del medio -> A5
                    patas de los lados -> 5V y GND
     LED (con resistor) -> pin 3
  ==========================================================
*/

const int PIN_POT = A5;
const int PIN_LED = 3;

const unsigned long PARPADEO_RAPIDO_MS = 40;    // perilla a un extremo
const unsigned long PARPADEO_LENTO_MS  = 1000;  // perilla al otro extremo

bool ledEncendido = false;
unsigned long ultimoCambio = 0;

void setup() {
  pinMode(PIN_LED, OUTPUT);
}

void loop() {
  int lectura = analogRead(PIN_POT);
  unsigned long intervalo = map(lectura, 0, 1023, PARPADEO_RAPIDO_MS, PARPADEO_LENTO_MS);

  // Se usa millis() en vez de delay() para que el cambio de la perilla
  // se note al tiro, sin esperar a que termine el parpadeo anterior.
  if (millis() - ultimoCambio >= intervalo) {
    ultimoCambio = millis();
    ledEncendido = !ledEncendido;
    digitalWrite(PIN_LED, ledEncendido ? HIGH : LOW);
  }
}
