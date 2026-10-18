/*
  ==========================================================
   SERVO LOCO  (Nivel 3)
   El potenciometro mueve el servo y la pantalla LCD muestra
   el angulo actual con una barra.
  ==========================================================
   Conexiones (segun el plano):
     Servo: senal (naranjo) -> pin 3, rojo -> 5V, cafe -> GND
     Potenciometro: pata del medio -> A0, lados -> 5V y GND
     Pantalla LCD I2C: GND -> GND, VCC -> 5V, SDA -> A4, SCL -> A5

   Librerias: "Servo" (ya viene con Arduino) y
              "LiquidCrystal I2C" de Frank de Brabander
   (Herramientas -> Administrar bibliotecas -> buscar "LiquidCrystal I2C")
   La direccion de la pantalla (0x27, 0x3F, etc.) se detecta sola.
  ==========================================================
*/

#include <Wire.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

const int PIN_SERVO = 3;
const int PIN_POT   = A0;

Servo servo;
LiquidCrystal_I2C *lcd;
int anguloActual = -100;

// Bloques para la barra (1 a 5 columnas pintadas)
byte barra1[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
byte barra2[8] = {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18};
byte barra3[8] = {0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C};
byte barra4[8] = {0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E};
byte barra5[8] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};

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
  lcd->createChar(1, barra1);
  lcd->createChar(2, barra2);
  lcd->createChar(3, barra3);
  lcd->createChar(4, barra4);
  lcd->createChar(5, barra5);
  lcd->clear();
}

int leerPotenciometro() {
  long suma = 0;
  for (byte i = 0; i < 8; i++) suma += analogRead(PIN_POT);   // promedio para que no tiemble
  return (int)(suma / 8);
}

void mostrarAngulo(int angulo) {
  char buf[17];
  lcd->setCursor(0, 0);
  snprintf(buf, sizeof(buf), "Angulo: %3d", angulo);
  lcd->print(buf);
  lcd->write((uint8_t)0xDF);   // simbolo de grados
  lcd->print("    ");

  int pixeles = map(angulo, 0, 180, 0, 80);   // 16 casillas x 5 columnas
  lcd->setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    int resto = pixeles - i * 5;
    if (resto >= 5)     lcd->write((uint8_t)5);
    else if (resto > 0) lcd->write((uint8_t)resto);
    else                lcd->write((uint8_t)' ');
  }
}

void setup() {
  servo.attach(PIN_SERVO);
  iniciarLCD();
  lcd->setCursor(0, 0);
  lcd->print("   SERVO LOCO");
  lcd->setCursor(0, 1);
  lcd->print("Mueve la perilla");
  delay(1500);
  lcd->clear();
}

void loop() {
  int angulo = map(leerPotenciometro(), 0, 1023, 0, 180);

  // Solo actualiza si cambio al menos 2 grados (evita temblores),
  // o si llego justo a un extremo.
  bool extremo = (angulo == 0 || angulo == 180) && angulo != anguloActual;
  if (abs(angulo - anguloActual) >= 2 || extremo) {
    anguloActual = angulo;
    servo.write(angulo);
    mostrarAngulo(angulo);
  }
  delay(20);
}
