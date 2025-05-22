#include "main.h"
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
}