/*
  ==========================================================
   RADAR SIMPLE  (Nivel 2)
   El sensor ultrasonico mide la distancia, la pantalla LCD la
   muestra y el buzzer pita mas rapido mientras mas cerca hay algo.
  ==========================================================
   Conexiones (segun el plano):
     Sensor HC-SR04: VCC -> 5V, GND -> GND, TRIG -> pin 3, ECHO -> pin 2
     Buzzer (+) -> pin 5,  Buzzer (-) -> GND
     Pantalla LCD I2C: GND -> GND, VCC -> 5V, SDA -> A4, SCL -> A5

   Libreria necesaria: "LiquidCrystal I2C" de Frank de Brabander
   (Herramientas -> Administrar bibliotecas -> buscar "LiquidCrystal I2C")
   La direccion de la pantalla (0x27, 0x3F, etc.) se detecta sola.
  ==========================================================
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int PIN_TRIG   = 3;
const int PIN_ECHO   = 2;
const int PIN_BUZZER = 5;

const int DISTANCIA_AVISO   = 100;  // cm: desde aqui empieza a pitar
const int DISTANCIA_PELIGRO = 10;   // cm: pitido continuo
const int TONO_HZ           = 2000;

LiquidCrystal_I2C *lcd;

int distancia = -1;          // -1 = no hay nada al frente
byte lecturasFallidas = 0;
bool sonandoContinuo = false;
unsigned long ultimaMedicion = 0;
unsigned long ultimaPantalla = 0;
unsigned long ultimoPitido   = 0;

// Busca la pantalla en el bus I2C, asi da lo mismo su direccion
byte buscarLCD() {
  for (byte dir = 0x08; dir < 0x78; dir++) {
    Wire.beginTransmission(dir);
    if (Wire.endTransmission() == 0) return dir;
  }
  return 0x27;   // por si acaso
}

void iniciarLCD() {
  Wire.begin();
  lcd = new LiquidCrystal_I2C(buscarLCD(), 16, 2);
  lcd->init();
  lcd->backlight();
}

int medirDistancia() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  unsigned long duracion = pulseIn(PIN_ECHO, HIGH, 25000UL);  // maximo ~4 metros
  if (duracion == 0) return -1;
  return (int)(duracion / 58);   // microsegundos -> centimetros
}

void actualizarPantalla() {
  char linea[17];
  lcd->setCursor(0, 0);
  if (distancia < 0) snprintf(linea, sizeof(linea), "Distancia: ---  ");
  else               snprintf(linea, sizeof(linea), "Distancia:%3dcm ", distancia);
  lcd->print(linea);

  lcd->setCursor(0, 1);
  if (distancia < 0 || distancia > DISTANCIA_AVISO) {
    lcd->print("Todo despejado  ");
  } else if (distancia <= DISTANCIA_PELIGRO) {
    lcd->print("!! MUY CERCA !! ");
  } else {
    // Barra: mientras mas cerca, mas larga
    int bloques = map(distancia, DISTANCIA_PELIGRO, DISTANCIA_AVISO, 16, 1);
    for (int i = 0; i < 16; i++) lcd->write(i < bloques ? (uint8_t)0xFF : (uint8_t)' ');
  }
}

void actualizarBuzzer() {
  // Muy cerca: pitido continuo
  if (distancia >= 0 && distancia <= DISTANCIA_PELIGRO) {
    if (!sonandoContinuo) {
      tone(PIN_BUZZER, TONO_HZ);
      sonandoContinuo = true;
    }
    return;
  }
  if (sonandoContinuo) {
    noTone(PIN_BUZZER);
    sonandoContinuo = false;
  }
  // Nada cerca: silencio
  if (distancia < 0 || distancia > DISTANCIA_AVISO) return;

  // Entre medio: pitidos, mas seguidos mientras mas cerca
  unsigned long pausa = map(distancia, DISTANCIA_PELIGRO, DISTANCIA_AVISO, 80, 700);
  if (millis() - ultimoPitido >= pausa) {
    ultimoPitido = millis();
    tone(PIN_BUZZER, TONO_HZ, 50);
  }
}

void setup() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  iniciarLCD();
  lcd->setCursor(0, 0);
  lcd->print("  RADAR SIMPLE  ");
  lcd->setCursor(0, 1);
  lcd->print("  iniciando...  ");
  delay(1200);
  lcd->clear();
}

void loop() {
  if (millis() - ultimaMedicion >= 60) {
    ultimaMedicion = millis();
    int d = medirDistancia();
    if (d >= 0) {
      distancia = d;
      lecturasFallidas = 0;
    } else if (++lecturasFallidas >= 3) {   // ignora fallas sueltas del sensor
      distancia = -1;
    }
  }

  actualizarBuzzer();

  if (millis() - ultimaPantalla >= 250) {
    ultimaPantalla = millis();
    actualizarPantalla();
  }
}
