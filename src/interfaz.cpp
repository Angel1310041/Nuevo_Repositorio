/*#include "main.h"
#include "interfaz.h"
#include "TransmisorRf.h"
#include "Pantalla.h"

IPAddress local_IP(192, 168, 8, 28);
IPAddress gateway(192, 168, 14, 1);
IPAddress subnet(255, 255, 255, 0);

AsyncWebServer server(80);

void entrarModoProgramacion() {
  imprimir("Entrando a modo programación...");
  modoprog = true;
  esp_task_wdt_reset();
  digitalWrite(LED_PIN, HIGH);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  imprimir("Activando Modo Programación...");
  xTaskCreatePinnedToCore(
    endpointsMProg,
    "endpoints",
    8192,
    NULL,
    2,
    NULL,
    0
  );
  imprimir("---# Modo Programación Activado #---", "verde");
}
void endpointsMProg(void *pvParameters) {
  server.on("/programacion", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Modo Programación Activado");
  });
  server.begin();
  while (true) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}*/
#include "main.h"
#include "interfaz.h"
#include "TransmisorRf.h"
#include "Pantalla.h"
#include <SPIFFS.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <esp_system.h>

IPAddress local_IP(192, 168, 8, 28);
IPAddress gateway(192, 168, 14, 1);
IPAddress subnet(255, 255, 255, 0);

const char* ssidAP = "MiInterfazAP";
const char* passwordAP = "12345678";

AsyncWebServer server(80);

extern String Version;

void animacionCarga() {
    const char* estados[] = {"-", "\\", "|", "/"};
    for (int i = 0; i < 10; i++) {
        Serial.print("\rCargando... ");
        Serial.print(estados[i % 4]);
        delay(200);
    }
    Serial.println("\rCargando... ¡Listo!");
}

void procesarArchivoJSON(const char* path, AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(256);
    doc["version"] = Version;

    File file = SPIFFS.open(path, "r");
    if (!file) {
        imprimir("No se pudo abrir el archivo JSON para leer");
        doc["error"] = "No se pudo abrir el archivo";
        String respuesta;
        serializeJson(doc, respuesta);
        request->send(500, "application/json", respuesta);
        return;
    }

    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error) {
        imprimir("Error al parsear el JSON");
        doc["error"] = "JSON no válido";
        String respuesta;
        serializeJson(doc, respuesta);
        request->send(400, "application/json", respuesta);
        return;
    }

    // Si todo está bien, responde con éxito y la versión
    imprimir("JSON recibido y procesado correctamente");
    doc["mensaje"] = "Archivo recibido y procesado";
    String respuesta;
    serializeJson(doc, respuesta);
    request->send(200, "application/json", respuesta);
}


void endpointsMProg(void *pvParameters) {
    animacionCarga();
    WiFi.mode(WIFI_AP);

    if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
        imprimir("Error al configurar la IP estática");
        vTaskDelete(NULL);
        return;
    }
    WiFi.softAP(ssidAP, passwordAP, 6, 0, 4);
    IPAddress IP = WiFi.softAPIP();
    imprimir("Punto de acceso creado: " + IP.toString());

    server.on("/programacion", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Modo Programación Activado");
    });

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!SPIFFS.exists("/interfaz.html.gz")) {
            imprimir("Archivo /interfaz.html.gz no encontrado en SPIFFS");
            request->send(404, "text/plain", "Archivo de interfaz no encontrado");
            return;
        }
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/interfaz.html.gz", "text/html");
        response->addHeader("Content-encoding", "gzip");
        request->send(response);
    });

    server.on("/reiniciar", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Reiniciando...");
    delay(100); // Permite que la respuesta se envíe antes de reiniciar
    ESP.restart();
});

    // <-- AQUÍ va tu endpoint LoRa
    server.on("/enviar-lora", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(400, "application/json", "{\"error\": \"Falta el cuerpo del mensaje\"}");
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, data, len);
        if (error) {
            request->send(400, "application/json", "{\"error\": \"JSON no válido\"}");
            return;
        }
        String mensaje = doc["mensaje"] | "";
        if (mensaje == "") {
            request->send(400, "application/json", "{\"error\": \"Falta el campo 'mensaje'\"}");
            return;
        }
        enviarPorLora(mensaje);
        request->send(200, "application/json", "{\"status\": \"Mensaje enviado por LoRa\"}");
    });

    server.begin();
    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void entrarModoProgramacion() {
    imprimir("Entrando a modo programación...");
    modoprog = true;
    esp_task_wdt_reset();
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    imprimir("Activando Modo Programación...");
    xTaskCreatePinnedToCore(
        endpointsMProg,
        "endpoints",
        8192,
        NULL,
        2,
        NULL,
        0
    );
    imprimir("---# Modo Programación Activado #---", "verde");
}

void entrarmodoprog() {
    if (!SPIFFS.begin(true)) {
        imprimir("Error al montar SPIFFS");
        return;
    }
    entrarModoProgramacion();
}