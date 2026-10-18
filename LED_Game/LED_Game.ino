/*
  ==========================================================
   LED GAME - CINCHADA  (Nivel 3)
   Juego para 2: la luz parte en el LED del medio.
   Cada vez que aprietas TU boton, la luz se tira hacia tu lado.
   El primero que lleve la luz hasta su extremo, gana!
   (Aprieta lo mas rapido que puedas)
  ==========================================================
   Conexiones (segun el plano):
     LEDs de izquierda a derecha -> pines 8, 7, 6, 5, 4
     (cada LED con su resistor a GND)
     Boton izquierdo -> pin 2, la otra pata a GND
     Boton derecho   -> pin 3, la otra pata a GND
     (no necesitan resistor, se usa INPUT_PULLUP)
  ==========================================================
*/

const int LEDS[5]    = {8, 7, 6, 5, 4};   // de izquierda a derecha
const int BOTONES[2] = {2, 3};            // [0] jugador izquierdo, [1] jugador derecho

const int META = 12;   // ventaja (en apretones) para ganar. Mas grande = partida mas larga
const int APRETONES_POR_LED = META / 3;

int cuerda = 0;        // negativo = gana la izquierda, positivo = gana la derecha

bool estadoBoton[2] = {HIGH, HIGH};
unsigned long tiempoBoton[2] = {0, 0};

// Devuelve true solo en el momento en que se aprieta el boton i
bool presionado(byte i) {
  bool lectura = digitalRead(BOTONES[i]);
  if (lectura != estadoBoton[i] && millis() - tiempoBoton[i] > 25) {
    estadoBoton[i] = lectura;
    tiempoBoton[i] = millis();
    return lectura == LOW;
  }
  return false;
}

void sincronizarBotones() {
  for (byte i = 0; i < 2; i++) {
    estadoBoton[i] = digitalRead(BOTONES[i]);
    tiempoBoton[i] = millis();
  }
}

void soloLed(int indice) {
  for (int i = 0; i < 5; i++) digitalWrite(LEDS[i], i == indice ? HIGH : LOW);
}

void todos(bool encendido) {
  for (int i = 0; i < 5; i++) digitalWrite(LEDS[i], encendido ? HIGH : LOW);
}

void mostrarCuerda() {
  int indice = 2 + cuerda / APRETONES_POR_LED;
  indice = constrain(indice, 0, 4);
  soloLed(indice);
}

void celebrar(byte ganador) {   // 0 = izquierda, 1 = derecha
  int ledGanador = (ganador == 0) ? 0 : 4;
  for (int i = 0; i < 8; i++) {
    soloLed(ledGanador); delay(80);
    todos(false);        delay(80);
  }
  // Olas hacia el lado del ganador
  for (int vuelta = 0; vuelta < 3; vuelta++) {
    if (ganador == 0) { for (int i = 4; i >= 0; i--) { soloLed(i); delay(60); } }
    else              { for (int i = 0; i <= 4; i++) { soloLed(i); delay(60); } }
  }
  todos(false);
  delay(400);
}

void cuentaRegresiva() {
  todos(true);                                                     delay(600);
  digitalWrite(LEDS[0], LOW); digitalWrite(LEDS[4], LOW);         delay(600);
  digitalWrite(LEDS[1], LOW); digitalWrite(LEDS[3], LOW);         delay(600);
  for (int i = 0; i < 3; i++) {
    digitalWrite(LEDS[2], LOW);  delay(80);
    digitalWrite(LEDS[2], HIGH); delay(80);
  }
  cuerda = 0;                    // YA!
  mostrarCuerda();
  sincronizarBotones();
}

void setup() {
  for (int i = 0; i < 5; i++) pinMode(LEDS[i], OUTPUT);
  for (int i = 0; i < 2; i++) pinMode(BOTONES[i], INPUT_PULLUP);
  cuentaRegresiva();
}

void loop() {
  if (presionado(0)) cuerda--;   // izquierda tira para su lado
  if (presionado(1)) cuerda++;   // derecha tira para su lado

  if (cuerda <= -META) { celebrar(0); cuentaRegresiva(); return; }
  if (cuerda >=  META) { celebrar(1); cuentaRegresiva(); return; }

  mostrarCuerda();
}
