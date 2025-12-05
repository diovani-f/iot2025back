#include "Botao.h"
#include <Arduino.h>

Botao::Botao(int pin, String topic_base, PubSubClient* mqttClient) {
    _pin = pin;
    _client = mqttClient;
    _topic = topic_base + "/" + String(pin); 
    _estadoAnterior = HIGH; // Assume PULLUP
}

void Botao::setup() {
    pinMode(_pin, INPUT_PULLUP);
    _estadoAnterior = digitalRead(_pin);
    Serial.printf("[Botao] Sensor inicializado no pino %d. Publicando em %s\n", _pin, _topic.c_str());
}

void Botao::loop() {
    int estadoAtual = digitalRead(_pin);

    if (_estadoAnterior == HIGH && estadoAtual == LOW) {
        Serial.printf("[Botao] Pino %d pressionado!\n", _pin);
        
        _client->publish(_topic.c_str(), "pressionado");
        delay(50);
    }
    _estadoAnterior = estadoAtual;
}

String Botao::getType() {
    return "botao";
}