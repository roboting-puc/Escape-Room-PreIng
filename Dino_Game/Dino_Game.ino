/*
  ==========================================================
   DINO GAME  (Nivel 2)
   El clasico juego del dinosaurio de Chrome en la pantalla
   LCD 16x2. El boton hace saltar al dino: esquiva los cactus!
   Mientras mas puntos, mas rapido va.
  ==========================================================
   Conexiones (segun el plano):
     Pantalla LCD I2C: GND -> GND, VCC -> 5V, SDA -> SDA, SCL -> SCL
     Boton: una pata -> pin 3, la otra -> GND
            (no necesita resistor, se usa INPUT_PULLUP)

   Libreria necesaria: "LiquidCrystal I2C" de Frank de Brabander
   (Herramientas -> Administrar bibliotecas -> buscar "LiquidCrystal I2C")
   La direccion de la pantalla (0x27, 0x3F, etc.) se detecta sola.
  ==========================================================
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int PIN_BOTON = 3;

const byte COL_DINO       = 1;    // columna donde corre el dino
const byte TICKS_SALTO    = 4;    // duracion del salto (en pasos del juego)
const int  VEL_INICIAL    = 260;  // ms por paso al comenzar (mas grande = mas lento)
const int  VEL_MAXIMA     = 90;   // ms por paso cuando va lo mas rapido posible
const byte SEPARACION_MIN = 4;    // espacio minimo entre cactus (no bajar de 4)
const byte SEPARACION_MAX = 10;   // espacio maximo entre cactus

// ---------- Dibujos de 5x8 pixeles ----------
byte dinoPaso1[8] = {
  0b00111, 0b00101, 0b00111, 0b10110,
  0b11111, 0b01110, 0b01010, 0b01000
};
byte dinoPaso2[8] = {
  0b00111, 0b00101, 0b00111, 0b10110,
  0b11111, 0b01110, 0b01010, 0b00010
};
byte dinoSalto[8] = {
  0b00111, 0b00101, 0b00111, 0b10110,
  0b11111, 0b01110, 0b01010, 0b00000
};
byte cactusGrande[8] = {
  0b00100, 0b00101, 0b10101, 0b10111,
  0b11100, 0b00100, 0b00100, 0b00100
};
byte cactusChico[8] = {
  0b00000, 0b00000, 0b00100, 0b10101,
  0b10111, 0b11100, 0b00100, 0b00100
};

#define CH_DINO1    1
#define CH_DINO2    2
#define CH_SALTO    3
#define CH_CACTUS1  4
#define CH_CACTUS2  5

#define ESTADO_INICIO   0
#define ESTADO_JUGANDO  1
#define ESTADO_PERDIO   2

LiquidCrystal_I2C *lcd;

byte estado = ESTADO_INICIO;
byte suelo[16];              // 0 = vacio, si no = cactus
byte enAire = 0;             // pasos que le quedan al dino en el aire
byte faltaParaCactus = 0;
unsigned int puntos = 0;
unsigned int record = 0;
bool patita = false;         // animacion de correr
unsigned long ultimoPaso = 0;
unsigned long tiempoPerdio = 0;

bool botonEstado = HIGH;
unsigned long botonTiempo = 0;

// ---------- Pantalla ----------
byte buscarLCD() {
  for (byte dir = 0x08; dir < 0x78; dir++) {
    Wire.beginTransmission(dir);
    if (Wire.endTransmission() == 0) return dir;
  }
  return 0x27;
}

void iniciarLCD() {
  Wire.begin();
  lcd = new LiquidCrystal_I2C(buscarLCD(), 16, 2);
  lcd->init();
  lcd->backlight();
  lcd->createChar(CH_DINO1,   dinoPaso1);
  lcd->createChar(CH_DINO2,   dinoPaso2);
  lcd->createChar(CH_SALTO,   dinoSalto);
  lcd->createChar(CH_CACTUS1, cactusGrande);
  lcd->createChar(CH_CACTUS2, cactusChico);
  lcd->clear();
}

// Devuelve true solo en el momento en que se aprieta el boton
bool botonPresionado() {
  bool lectura = digitalRead(PIN_BOTON);
  if (lectura != botonEstado && millis() - botonTiempo > 30) {
    botonEstado = lectura;
    botonTiempo = millis();
    return lectura == LOW;
  }
  return false;
}

uint8_t letraDino() {
  if (enAire > 0) return CH_SALTO;
  return patita ? CH_DINO1 : CH_DINO2;
}

void dibujarDino() {
  lcd->setCursor(COL_DINO, 0);
  lcd->write(enAire > 0 ? letraDino() : (uint8_t)' ');
  lcd->setCursor(COL_DINO, 1);
  if (enAire > 0) lcd->write(suelo[COL_DINO] != 0 ? suelo[COL_DINO] : (uint8_t)' ');
  else            lcd->write(letraDino());
}

void dibujarTodo() {
  // Fila de abajo: el suelo con los cactus
  lcd->setCursor(0, 1);
  for (byte i = 0; i < 16; i++) {
    if (suelo[i] != 0) lcd->write(suelo[i]);
    else               lcd->write((uint8_t)' ');
  }
  // Puntaje arriba a la derecha
  char buf[7];
  snprintf(buf, sizeof(buf), "%5u", puntos);
  lcd->setCursor(11, 0);
  lcd->print(buf);
  // El dino al final, encima de todo
  dibujarDino();
}

// ---------- Juego ----------
void pantallaInicio() {
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print("   DINO  GAME");
  lcd->setCursor(0, 1);
  lcd->write((uint8_t)CH_DINO1);
  lcd->print(" Boton: jugar");
  lcd->setCursor(15, 1);
  lcd->write((uint8_t)CH_CACTUS1);
}

void empezar() {
  for (byte i = 0; i < 16; i++) suelo[i] = 0;
  enAire = 0;
  faltaParaCactus = 8;     // pista libre al comienzo
  puntos = 0;
  randomSeed(micros());
  lcd->clear();
  estado = ESTADO_JUGANDO;
  ultimoPaso = millis();
  dibujarTodo();
}

void perder() {
  estado = ESTADO_PERDIO;
  if (puntos > record) record = puntos;

  // La pantalla parpadea
  for (byte i = 0; i < 3; i++) {
    lcd->noBacklight(); delay(120);
    lcd->backlight();   delay(120);
  }

  char buf[17];
  lcd->clear();
  lcd->setCursor(0, 0);
  snprintf(buf, sizeof(buf), "GAME OVER! %5u", puntos);
  lcd->print(buf);
  lcd->setCursor(0, 1);
  snprintf(buf, sizeof(buf), "Record:    %5u", record);
  lcd->print(buf);

  tiempoPerdio = millis();
}

unsigned long velocidad() {
  long v = (long)VEL_INICIAL - (long)(puntos / 3);
  if (v < VEL_MAXIMA) v = VEL_MAXIMA;
  return (unsigned long)v;
}

void avanzar() {
  // Los cactus avanzan una casilla a la izquierda
  for (byte i = 0; i < 15; i++) suelo[i] = suelo[i + 1];

  if (faltaParaCactus == 0) {
    suelo[15] = (random(2) == 0) ? CH_CACTUS1 : CH_CACTUS2;
    faltaParaCactus = random(SEPARACION_MIN, SEPARACION_MAX + 1);
  } else {
    suelo[15] = 0;
    faltaParaCactus--;
  }

  if (enAire > 0) enAire--;

  // Choque?
  if (suelo[COL_DINO] != 0 && enAire == 0) {
    perder();
    return;
  }

  puntos++;
  patita = !patita;
  dibujarTodo();
}

void setup() {
  pinMode(PIN_BOTON, INPUT_PULLUP);
  randomSeed(analogRead(A0));
  iniciarLCD();
  pantallaInicio();
}

void loop() {
  bool presion = botonPresionado();

  if (estado == ESTADO_INICIO) {
    if (presion) empezar();
  }
  else if (estado == ESTADO_JUGANDO) {
    if (presion && enAire == 0) {
      enAire = TICKS_SALTO;
      dibujarDino();           // salta al tiro, sin esperar el siguiente paso
    }
    if (millis() - ultimoPaso >= velocidad()) {
      ultimoPaso = millis();
      avanzar();
    }
  }
  else {  // ESTADO_PERDIO
    if (presion && millis() - tiempoPerdio > 600) empezar();
  }
}
