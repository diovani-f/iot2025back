#include "LedAtuador.h"
#include <Arduino.h>
#include <ArduinoJson.h>

LedAtuador::LedAtuador(int pin, String topic_base, PubSubClient* mqttClient) {
    _pin = pin;
    _client = mqttClient;
    _controlTopic = topic_base + "/" + String(pin);
}

void LedAtuador::setup() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    
    Serial.printf("[LED] Atuador inicializado no pino %d.\n", _pin);
    Serial.printf("[LED] Ouvindo no tópico: %s\n", _controlTopic.c_str());
}

void LedAtuador::loop() {}

String LedAtuador::getType() {
    return "led";
}

void LedAtuador::handleMqttMessage(String topic, String payload) {
    if (topic != _controlTopic) return;

    Serial.printf("[LED] Pino %d - Comando recebido: %s\n", _pin, payload.c_str());

    String estadoReportado = "ERRO";

    if (payload == "ON") {
        digitalWrite(_pin, HIGH);
        estadoReportado = "LIGADO";
        
    } else if (payload == "OFF") {
        digitalWrite(_pin, LOW);
        estadoReportado = "DESLIGADO";

    } else {
        Serial.println("[LED] Comando desconhecido. Use 'ON' ou 'OFF'.");
        estadoReportado = "COMANDO_INVALIDO";
    }

    DynamicJsonDocument doc(128);
    doc["status"] = "OK";
    doc["estado"] = estadoReportado;

    char jsonPayload[128];
    serializeJson(doc, jsonPayload, sizeof(jsonPayload));

    String estadoTopic = _controlTopic + "/estado";
    if (_client->connected()) {
        _client->publish(estadoTopic.c_str(), jsonPayload);
    }
}