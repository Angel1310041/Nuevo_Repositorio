#include <Pantalla.h>
#include <main.h>
#include <interfaz.h>


String Version = "1.1.1.1";

void blinkLed() {
  const int ledPin = LED_PIN;
  const int delayTime = 250; 

  digitalWrite(ledPin, HIGH);
  delay(delayTime);
  digitalWrite(ledPin, LOW);
  delay(delayTime);
}

void setup() {
  pinMode(LED_PIN, OUTPUT); 
  
  Heltec.begin(true, false, true);
}

void loop() {
  blinkLed();
  alternarImagen(); 
  delay(1000); 
}