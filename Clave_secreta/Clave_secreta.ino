/*
  ==========================================================
   CLAVE SECRETA  (Nivel 3)
   Cada vez que aprietas un boton los LEDs hacen combinaciones
   raras. Si aprietas el boton correcto, avanzas; si te
   equivocas, parpadean todos y vuelves a empezar.
   Descubre la combinacion que deja TODOS los LEDs encendidos!

   Para reiniciar: manten AMBOS botones apretados 2 segundos.
  ==========================================================
   Conexiones (segun el plano):
     LEDs de izquierda a derecha -> pines 7, 6, 4, 5
     (cada LED con su resistor a GND)
     Boton 1 (izquierda) -> pin 2, la otra pata a GND
     Boton 2 (derecha)   -> pin 3, la otra pata a GND
     (no necesitan resistor, se usa INPUT_PULLUP)
  ==========================================================
*/

const int LEDS[4]    = {7, 6, 4, 5};   // de izquierda a derecha
const int BOTONES[2] = {2, 3};         // boton 1, boton 2

// ============ LA CLAVE SECRETA ============
// 1 = boton 1,  2 = boton 2.  Puedes cambiarla, alargarla o acortarla.
const byte CLAVE[] = {1, 2, 2, 1, 2};
const byte LARGO_CLAVE = sizeof(CLAVE) / sizeof(CLAVE[0]);

// Combinaciones "raras" que se muestran al acertar cada paso
// (cada bit es un LED: 0b1001 = el primero y el ultimo encendidos)
const byte PATRONES[] = {0b1001, 0b0110, 0b1010, 0b0101, 0b1101, 0b0011, 0b1110, 0b0111};
const byte NUM_PATRONES = sizeof(PATRONES) / sizeof(PATRONES[0]);

byte paso = 0;
bool abierto = false;
unsigned long inicioAmbos = 0;

bool estadoBoton[2] = {HIGH, HIGH};
unsigned long tiempoBoton[2] = {0, 0};

// Devuelve true solo en el momento en que se aprieta el boton i
bool presionado(byte i) {
  bool lectura = digitalRead(BOTONES[i]);
  if (lectura != estadoBoton[i] && millis() - tiempoBoton[i] > 30) {
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

void mostrar(byte patron) {
  for (byte i = 0; i < 4; i++) {
    digitalWrite(LEDS[i], ((patron >> i) & 1) ? HIGH : LOW);
  }
}

void destelloRaro() {
  for (byte i = 0; i < 6; i++) {
    mostrar((byte)random(1, 16));
    delay(70);
  }
}

void animacionError() {
  for (byte i = 0; i < 4; i++) {
    mostrar(0b1111); delay(90);
    mostrar(0);      delay(90);
  }
  delay(200);
}

void animacionVictoria() {
  for (byte vuelta = 0; vuelta < 3; vuelta++) {
    for (int i = 0; i < 4; i++)  { mostrar(1 << i); delay(70); }
    for (int i = 2; i >= 1; i--) { mostrar(1 << i); delay(70); }
  }
  for (byte i = 0; i < 3; i++) {
    mostrar(0b1111); delay(200);
    mostrar(0);      delay(150);
  }
  mostrar(0b1111);   // y se quedan encendidos!
}

void reiniciar() {
  paso = 0;
  abierto = false;
  for (int i = 0; i < 4; i++)  { mostrar(1 << i); delay(100); }
  for (int i = 3; i >= 0; i--) { mostrar(1 << i); delay(100); }
  mostrar(0);
}

void setup() {
  for (byte i = 0; i < 4; i++) pinMode(LEDS[i], OUTPUT);
  for (byte i = 0; i < 2; i++) pinMode(BOTONES[i], INPUT_PULLUP);
  randomSeed(analogRead(A0));
  reiniciar();
  sincronizarBotones();
}

void loop() {
  bool p1 = presionado(0);
  bool p2 = presionado(1);

  // Mantener AMBOS botones 2 segundos = reiniciar
  if (digitalRead(BOTONES[0]) == LOW && digitalRead(BOTONES[1]) == LOW) {
    if (inicioAmbos == 0) inicioAmbos = millis();
    if (millis() - inicioAmbos > 2000) {
      reiniciar();
      while (digitalRead(BOTONES[0]) == LOW || digitalRead(BOTONES[1]) == LOW) { }
      delay(50);
      sincronizarBotones();
      inicioAmbos = 0;
      return;
    }
  } else {
    inicioAmbos = 0;
  }

  if (abierto) return;   // ya la descubrieron: los LEDs se quedan prendidos

  byte boton = 0;
  if (p1)      boton = 1;
  else if (p2) boton = 2;
  if (boton == 0) return;

  if (boton == CLAVE[paso]) {
    paso++;
    if (paso >= LARGO_CLAVE) {
      abierto = true;
      animacionVictoria();
    } else {
      destelloRaro();
      mostrar(PATRONES[(paso - 1) % NUM_PATRONES]);
    }
  } else {
    animacionError();
    paso = 0;
    mostrar(0);
  }
  sincronizarBotones();
}
