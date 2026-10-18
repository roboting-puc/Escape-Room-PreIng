/*
  ==========================================================
   SIRENA POLICIAL  (Nivel 2)
   Dos LEDs (rojo y azul) destellan alternados mientras el
   buzzer sube y baja el tono, como una sirena de verdad.
  ==========================================================
   Conexiones (segun el plano):
     LED rojo (con resistor) -> pin 4
     LED azul (con resistor) -> pin 2
     Buzzer (+) -> pin 3,  Buzzer (-) -> GND
  ==========================================================
*/

const int PIN_ROJO   = 4;
const int PIN_AZUL   = 2;
const int PIN_BUZZER = 3;

const int TONO_BAJO = 650;    // Hz
const int TONO_ALTO = 1500;   // Hz
const int SALTO_HZ  = 8;      // que tan rapido sube/baja el tono
const int ESPERA_MS = 5;

void setup() {
  pinMode(PIN_ROJO, OUTPUT);
  pinMode(PIN_AZUL, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
}

void loop() {
  // Sube el tono -> destella el ROJO
  digitalWrite(PIN_AZUL, LOW);
  int paso = 0;
  for (int f = TONO_BAJO; f <= TONO_ALTO; f += SALTO_HZ) {
    tone(PIN_BUZZER, f);
    digitalWrite(PIN_ROJO, ((paso / 10) % 2 == 0) ? HIGH : LOW);  // efecto estrobo
    paso++;
    delay(ESPERA_MS);
  }

  // Baja el tono -> destella el AZUL
  digitalWrite(PIN_ROJO, LOW);
  paso = 0;
  for (int f = TONO_ALTO; f >= TONO_BAJO; f -= SALTO_HZ) {
    tone(PIN_BUZZER, f);
    digitalWrite(PIN_AZUL, ((paso / 10) % 2 == 0) ? HIGH : LOW);
    paso++;
    delay(ESPERA_MS);
  }
}
