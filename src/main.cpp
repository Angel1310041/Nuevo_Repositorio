#include "Pantalla.h"
#include "interfaz.h"
#include "TransmisorRf.h"
#include "main.h"
#include <RCSwitch.h>
#include <heltec.h>

String Version = "2.1.2.4";

const int EEPROM_SIZE = 512;
boolean debug = true, variableDetectada = false;
boolean modoprog = false; 
SENSOR activo{-1, -1, -1};

byte rowPins[ROWS] = {7, 6, 5};
byte colPins[COLS] = {4, 3, 2};
char keys[ROWS][COLS] = {{'1','2','3'},{'4','5','6'},{'7','8','9'}};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

extern const unsigned char img1[];
extern const unsigned char img2[];
extern const unsigned char img3[];
extern const unsigned char img4[];
extern const unsigned char img5[];
extern const unsigned char img6[];

// Matriz de tipo de sensor por botón (ajusta según tu lógica)
int matrizTipoSensor[ROWS][COLS] = {
  {0, 2, 3},
  {1, 4, 5},
  {6, 7, 0}
};

void imprimir(String m, String c=""){
  if(!debug) return;
  const char*col="\033[0m";
  if(c=="rojo") col="\033[31m";
  else if(c=="verde") col="\033[32m";
  else if(c=="amarillo") col="\033[33m";
  else if(c=="cyan") col="\033[36m";
  Serial.print(col); Serial.println(m); Serial.print("\033[0m");
}

// Muestra la imagen correspondiente según el tipo de sensor
void mostrarImagenPorTipoSensor(int tipoSensor) {
    switch (tipoSensor) {
        case 0:
        case 1:
            Heltec.display->clear();
            Heltec.display->drawXbm(0, 0, 128, 64, img3);
            Heltec.display->display();
            break;
        case 2:
            Heltec.display->clear();
            Heltec.display->drawXbm(0, 0, 128, 64, img5);
            Heltec.display->display();
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            Heltec.display->clear();
            Heltec.display->drawXbm(0, 0, 128, 64, img6);
            Heltec.display->display();
            break;
        case 9:
            Heltec.display->clear();
            Heltec.display->drawXbm(0, 0, 128, 64, img2);
            Heltec.display->display();
            break;
        default:
            break;
    }
}

// Muestra la imagen de inicio (LoRa conectada)
void mostrarInicio() {
    Heltec.display->clear();
    Heltec.display->drawXbm(0, 0, 128, 64, img1);
    Heltec.display->display();
}

void blinkLed() {
    for (int i = 0; i < 2; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(500);
        digitalWrite(LED_PIN, LOW);
        delay(500);
    }
}

void manejarEntradas() {
  static unsigned long t = 0, progStart = 0;
  static bool esperandoLiberar = false;
  int mq6 = digitalRead(MQ6_PIN), progEstado = digitalRead(prog);

  if (progEstado == LOW) {
    if (!progStart) progStart = millis();
    if (!modoprog && millis() - progStart >= 2000 && !esperandoLiberar) {
      modoprog = esperandoLiberar = true;
      digitalWrite(LED_PIN, HIGH);
      Heltec.display->clear();
      Heltec.display->drawXbm(0, 0, 128, 64, img4); 
      Heltec.display->display();
      imprimir("Entrando en modo programacion...", "cyan");
    }
  } else {
    progStart = 0; esperandoLiberar = false;
  }

  if (millis() - t > 10000) { imprimir("Lectura MQ-6: " + String(mq6)); t = millis(); }
  if (modoprog) return;

  if (mq6 == LOW && !variableDetectada) {
    enviarRF_Matriz(0, 0, 0, 0, 0);
    mostrarImagenPorTipoSensor(0); // img3 para tipo 0 (gas)
    blinkLed();
    variableDetectada = true;
    imprimir("¡Gas detectado! Alerta RF enviada.", "rojo");
  } else if (mq6 == HIGH) variableDetectada = false;

  if (char k = keypad.getKey()) {
    for (int r = 0; r < ROWS; r++)
      for (int c = 0; c < COLS; c++)
        if (keys[r][c] == k) {
          int tipoSensor = matrizTipoSensor[r][c];
          enviarRF_Matriz(r, c, 0, tipoSensor, 0);
          
          blinkLed();
        }
  }
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  pinMode(MQ6_PIN, INPUT); pinMode(LED_PIN, OUTPUT); pinMode(prog, INPUT_PULLUP);
  Heltec.begin(true,false,true);
  mostrarInicio();
  Transmisorrf.enableTransmit(33);
  EEPROM.get(0, activo);
  if(activo.id<0||activo.zona<0||activo.zona>10||activo.tipo<0||activo.tipo>8){
    imprimir("Datos EEPROM inválidos, restaurando...","amarillo");
    activo={0,0,0};
    EEPROM.put(0, activo);
    EEPROM.commit();
  }
  imprimir("ID: "+String(activo.id));
  imprimir("Zona: "+String(activo.zona));
  imprimir("Tipo: "+String(activo.tipo));
}

void loop() {
  manejarEntradas();
  
}