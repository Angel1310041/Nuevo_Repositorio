#include "main.h"
#include "interfaz.h"
#include "TransmisorRf.h"
#include "Pantalla.h"
#include <SPIFFS.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <esp_system.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Ticker.h>

IPAddress local_IP(192, 168, 8, 28);
IPAddress gateway(192, 168, 8, 1);
IPAddress subnet(255, 255, 255, 0);

const char* ssidAP = "MiInterfazAP";
const char* passwordAP = "12345678";

AsyncWebServer server(80);
Ticker restartTimer;
extern String Version;

String mensajePendiente = "";
bool enviarLoraPendiente = false;

void animacionCarga() {
    const char* estados[] = {"-", "\\", "|", "/"};
    for (int i = 0; i < 10; i++) {
        Serial.print("\rCargando... ");
        Serial.print(estados[i % 4]);
        vTaskDelay(200 / portTICK_PERIOD_MS);
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

    imprimir("JSON recibido y procesado correctamente");
    doc["mensaje"] = "Archivo recibido y procesado";
    String respuesta;
    serializeJson(doc, respuesta);
    request->send(200, "application/json", respuesta);
}

// Función segura para reiniciar
void programarReinicio() {
    restartTimer.once(1.0, []() {
        ESP.restart();
    });
}

void endpointsMProg(void *pvParameters) {
    animacionCarga();

    WiFi.mode(WIFI_AP);
    if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
        imprimir("Error al configurar la IP estática");
        vTaskDelete(NULL);
        return;
    }

    if (!WiFi.softAP(ssidAP, passwordAP, 6, 0, 4)) {
        imprimir("Error al iniciar el AP");
        vTaskDelete(NULL);
        return;
    }

    IPAddress IP = WiFi.softAPIP();
    imprimir("Punto de acceso creado: " + IP.toString());

    // Endpoints
    server.on("/programacion", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Modo Programación Activado");
    });

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!SPIFFS.exists("/interfaz.html.gz")) {
            imprimir("Archivo /interfaz.html.gz no encontrado");
            request->send(404, "text/plain", "Archivo no encontrado");
            return;
        }
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/interfaz.html.gz", "text/html");
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });

    server.on("/reiniciar", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Reiniciando...");
        programarReinicio(); // usa Ticker en vez de ESP.restart directo
    });

    server.on("/guardar-parametros", HTTP_POST,
[](AsyncWebServerRequest *request) {},
NULL,
[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    // Aumentamos el tamaño del documento JSON si es necesario, 256 puede ser justo
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, data, len);

    if (error) {
        request->send(400, "application/json", "{\"error\": \"JSON inválido: " + String(error.c_str()) + "\"}");
        return;
    }

    // Extraer los valores del JSON. Usamos get<int>() para asegurar que sean enteros.
    // Proporcionamos un valor por defecto (-1) para detectar si faltan.
    int nuevoId = doc["id-alarma"].as<int>();
    int nuevaZona = doc["zona"].as<int>();
    int nuevoTipo = doc["tipo-sensor"].as<int>();

    // Validar que los parámetros requeridos estén presentes y sean válidos (opcional pero recomendado)
    // Puedes añadir validaciones más específicas aquí si es necesario
    if (doc["id-alarma"].isNull() || doc["zona"].isNull() || doc["tipo-sensor"].isNull()) {
         request->send(400, "application/json", "{\"error\": \"Parámetros incompletos o inválidos\"}");
         return;
    }

    // Validar rangos (opcional, basado en tu formulario)
    if (nuevoId < 1000 || nuevoId > 9999 || nuevaZona < 1 || nuevaZona > 512 || nuevoTipo < 0 || nuevoTipo > 9) {
         request->send(400, "application/json", "{\"error\": \"Valores de parámetros fuera de rango\"}");
         return;
    }


    // Actualizar la variable global 'activo' con los nuevos valores
    // Asegúrate de que 'activo' es una variable global de tipo SENSOR
    activo.id = nuevoId;
    activo.zona = nuevaZona;
    activo.tipo = nuevoTipo;

    // Guardar la estructura 'activo' actualizada en la EEPROM en la dirección 0
    // Asegúrate de que EEPROM.begin(EEPROM_SIZE) con un tamaño suficiente
    // para la estructura SENSOR ya se llamó en setup().
    EEPROM.put(0, activo);

    // Confirmar la escritura en la EEPROM
    bool success = EEPROM.commit();

    if (success) {
        Serial.println("Parámetros guardados en EEPROM correctamente.");
        request->send(200, "application/json", "{\"status\": \"Parámetros guardados\"}");
    } else {
        Serial.println("Error al guardar parámetros en EEPROM.");
        request->send(500, "application/json", "{\"error\": \"Error al guardar en EEPROM\"}");
    }
});



    server.on("/enviar-lora", HTTP_POST,
    [](AsyncWebServerRequest *request) {
        request->send(400, "application/json", "{\"error\": \"Falta el cuerpo del mensaje\"}");
    },
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
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

        mensajePendiente = mensaje;
        enviarLoraPendiente = true;
        request->send(200, "application/json", "{\"status\": \"Mensaje recibido y será enviado\"}");
    });


    server.begin();

    vTaskDelay(100 / portTICK_PERIOD_MS);
    vTaskDelete(NULL);
}

void entrarModoProgramacion() {
    imprimir("Entrando a modo programación...");
    modoprog = true;
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
        1 
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

