#include "Pantalla.h"
#include "interfaz.h"
#include "TransmisorRf.h"
#include "main.h"
#include <RCSwitch.h>
#include <heltec.h>

String Version = "3.2.1.1";
const int EEPROM_SIZE = 512;

boolean debug = true, variableDetectada = false, modoprog = false; 
SENSOR activo{-1, -1, -1};

const int BOTON_PRUEBA_PIN = 2;
unsigned long tiempoUltimaImagen = 0;
int imagenMostrada = 1; 

extern String mensajePendiente;
extern bool enviarLoraPendiente;

extern const unsigned char img1[], img2[], img3[], img4[], img5[], img6[];

void imprimir(String m, String c) {
  if (!debug) return;
  const char* col = "\033[0m";
  if (c == "rojo") col = "\033[31m";
  else if (c == "verde") col = "\033[32m";
  else if (c == "amarillo") col = "\033[33m";
  else if (c == "cyan") col = "\033[36m";
  Serial.print(col); Serial.println(m); Serial.print("\033[0m");
}

void enviarPorLora(String mensaje) {
  LoRa.beginPacket();
  LoRa.print(mensaje);
  LoRa.endPacket();
  Serial.println("Lora enviado: " + mensaje);
}

void mostrarImagen(const unsigned char* imagen, int tipo = 2) {
  Heltec.display->clear();
  Heltec.display->drawXbm(0, 0, 128, 64, imagen);
  Heltec.display->display();
  if (tipo == 1) {
    imagenMostrada = 1;
  } else {
    imagenMostrada = 2;
    tiempoUltimaImagen = millis();
  }
}

void mostrarInicio() {
  mostrarImagen(img1, 1);
}

void mostrarImagenPorTipoSensor(int tipoSensor) {
  switch (tipoSensor) {
    case 0:
    case 1:
      mostrarImagen(img3);
      break;
    case 2:
      mostrarImagen(img5);
      break;
    case 3:
    case 4:
      mostrarImagen(img8);
      break;
    case 5:
    case 6:
     mostrarImagen(img6);
      break;
    case 7:
      mostrarImagen(img7);
      break;
    case 9:
      mostrarImagen(img2);
      break;
    default:
      break;
  }
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
  static bool botonAnterior = HIGH;

  int mq6 = digitalRead(MQ6_PIN);
  int progEstado = digitalRead(prog);
  int estadoBoton = digitalRead(BOTON_PRUEBA_PIN);

  if (progEstado == LOW) {
    if (!progStart) progStart = millis();
    if (!modoprog && millis() - progStart >= 2000 && !esperandoLiberar) {
      modoprog = esperandoLiberar = true;
      entrarmodoprog();
      mostrarImagen(img4);
      if (!modoprog) imprimir("Entrando en modo programacion...", "cyan");
    }
  } else {
    progStart = 0;
    esperandoLiberar = false;
  }

  if (!modoprog) {
    if (millis() - t > 10000) {
      imprimir("Lectura MQ-6: " + String(mq6));
      t = millis();
    }

    if (mq6 == LOW && !variableDetectada) {
      // *** Restaurado: Enviar por Transmisor RF (RCSwitch) ***
      //enviarRF_Matriz(0, 0, 0, 0, 0); // Si usabas esta, descoméntala
      Transmisorrf.send(33330001, 32); // Envía el código RF para gas

      // --- Código para LoRa (comentado o eliminado si solo quieres RF) ---
      // mensajePendiente = "ALARM:" + String(activo.id) + ":" + String(activo.zona) + ":" + String(activo.tipo) + ":GAS";
      // enviarLoraPendiente = true;
      // -----------------------------------------------------------------

      mostrarImagenPorTipoSensor(0); // Asumiendo que 0 es el tipo para gas
      blinkLed();
      variableDetectada = true;
      imprimir("¡Gas detectado! Alerta RF enviada.", "rojo"); // Mensaje ajustado
    } else if (mq6 == HIGH) {
      variableDetectada = false;
    }

    if (estadoBoton == LOW && botonAnterior == HIGH) {
      // *** Restaurado: Enviar por Transmisor RF (RCSwitch) ***
      Transmisorrf.send(33339001, 32); // Envía el código RF para botón de prueba

      // --- Código para LoRa (comentado o eliminado si solo quieres RF) ---
      // mensajePendiente = "ALARM:" + String(activo.id) + ":" + String(activo.zona) + ":" + String(activo.tipo) + ":BUTTON";
      // enviarLoraPendiente = true;
      // -----------------------------------------------------------------

      blinkLed();
      mostrarImagen(img2); // Asumiendo que img2 es para el botón de prueba
      imprimir("Código de prueba RF enviado: 33339001", "verde"); // Mensaje ajustado
    }
    botonAnterior = estadoBoton;
  }


  if (!modoprog && imagenMostrada == 2 && millis() - tiempoUltimaImagen >= 10000) {
    mostrarInicio();
  }
}



void procesarEnvioLora() {
  if (enviarLoraPendiente && mensajePendiente.length() > 0) {
    enviarPorLora(mensajePendiente);
    mensajePendiente = "";
    enviarLoraPendiente = false;
  }
}



void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE); // Inicialización de la EEPROM
  pinMode(MQ6_PIN, INPUT_PULLUP);
   pinMode(LED_PIN, INPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(prog, INPUT_PULLUP);
  pinMode(BOTON_PRUEBA_PIN, INPUT_PULLUP);

  Heltec.begin(true, false, true);
  mostrarInicio();

  Transmisorrf.enableTransmit(33);

  EEPROM.get(0, activo); // Lee la estructura SENSOR desde la dirección 0

  // *** Lógica de validación ajustada para coincidir con los rangos del formulario ***
  // Validar ID (1000-9999), Zona (1-512), Tipo (0-7, 9)
  if (activo.id < 1000 || activo.id > 9999 ||
      activo.zona < 1 || activo.zona > 512 ||
      activo.tipo < 0 || (activo.tipo > 7 && activo.tipo != 9)) // Tipo debe ser 0-7 o 9
  {
    imprimir("Datos EEPROM inválidos o fuera de rango, restaurando...", "amarillo");
    activo = {0, 0, 0}; // Restaura a valores por defecto
    EEPROM.put(0, activo); // Guarda los valores por defecto
    EEPROM.commit();
  }


  imprimir("ID: " + String(activo.id));
  imprimir("Zona: " + String(activo.zona));
  imprimir("Tipo: " + String(activo.tipo));
}

void loop() {
  manejarEntradas();
  procesarEnvioLora();
}
