#include "ReleAtuador.h"
#include <Arduino.h>

ReleAtuador::ReleAtuador(int pin, String topic_base, PubSubClient* mqttClient, bool invertido) {
    _pin = pin;
    _client = mqttClient;
    _invertido = invertido;
    _controlTopic = topic_base + "/" + String(pin);
}

void ReleAtuador::setup() {
    pinMode(_pin, OUTPUT);
    
    digitalWrite(_pin, _invertido ? HIGH : LOW);
    
    Serial.printf("[Rele] Atuador inicializado no pino %d (Invertido: %s).\n", _pin, _invertido ? "SIM" : "NAO");
    Serial.printf("[Rele] Ouvindo no tópico: %s\n", _controlTopic.c_str());
}

void ReleAtuador::loop() {}

String ReleAtuador::getType() {
    return "rele";
}

void ReleAtuador::handleMqttMessage(String topic, String payload) {
    if (topic != _controlTopic) return;

    Serial.printf("[Rele] Pino %d - Comando recebido: %s\n", _pin, payload.c_str());

    if (payload == "ON") {
        digitalWrite(_pin, _invertido ? LOW : HIGH);
        
        if (_client->connected()) _client->publish((_controlTopic + "/estado").c_str(), "LIGADO");

    } else if (payload == "OFF") {
        digitalWrite(_pin, _invertido ? HIGH : LOW);

        if (_client->connected()) _client->publish((_controlTopic + "/estado").c_str(), "DESLIGADO");

    } else {
        Serial.println("[Rele] Comando desconhecido. Use 'ON' ou 'OFF'.");
    }
}