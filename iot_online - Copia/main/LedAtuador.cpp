#include "LedAtuador.h"
#include <Arduino.h>
#include <ArduinoJson.h>

// --- Construtor ---
LedAtuador::LedAtuador(int pin, String topic_base, PubSubClient* mqttClient) {
    _pin = pin;
    _client = mqttClient;
    _controlTopic = topic_base + "/" + String(pin);
    _autoOffTime = 0; // 0 significa timer desativado
}

// --- Setup ---
void LedAtuador::setup() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    
    Serial.printf("[LED] Atuador inicializado no pino %d.\n", _pin);
    Serial.printf("[LED] Ouvindo no tópico: %s\n", _controlTopic.c_str());
}

// --- Loop (Lógica do Timer) ---
void LedAtuador::loop() {
    // Se o timer estiver ativo (> 0)
    if (_autoOffTime > 0) {
        // Verifica se o tempo já passou
        if (millis() > _autoOffTime) {
            // Desliga
            digitalWrite(_pin, LOW);
            _autoOffTime = 0; // Desativa o timer
            
            Serial.printf("[LED] Pino %d - Desligamento automatico (Timer).\n", _pin);

            // Avisa o MQTT que desligou
            if (_client->connected()) {
                DynamicJsonDocument doc(128);
                doc["status"] = "OK";
                doc["estado"] = "DESLIGADO";
                doc["motivo"] = "TIMER_FIM";
                char jsonPayload[128];
                serializeJson(doc, jsonPayload, sizeof(jsonPayload));
                
                String estadoTopic = _controlTopic + "/estado";
                _client->publish(estadoTopic.c_str(), jsonPayload);
            }
        }
    }
}

// --- getType ---
String LedAtuador::getType() {
    return "led";
}

// --- Handle Message ---
void LedAtuador::handleMqttMessage(String topic, String payload) {
    if (topic != _controlTopic) return;

    Serial.printf("[LED] Pino %d - Comando recebido: %s\n", _pin, payload.c_str());

    String estadoReportado = "ERRO";

    if (payload == "ON") {
        digitalWrite(_pin, HIGH);
        _autoOffTime = 0; // Cancela timer (fica ligado fixo)
        estadoReportado = "LIGADO";
        
    } else if (payload == "OFF") {
        digitalWrite(_pin, LOW);
        _autoOffTime = 0; // Cancela timer
        estadoReportado = "DESLIGADO";

    // --- NOVOS COMANDOS ---
    } else if (payload == "ON_1S") {
        digitalWrite(_pin, HIGH);
        _autoOffTime = millis() + 1000; // Desliga daqui 1000ms
        estadoReportado = "LIGADO_TIMER_1S";

    } else if (payload == "ON_3S") {
        digitalWrite(_pin, HIGH);
        _autoOffTime = millis() + 3000; // Desliga daqui 3000ms
        estadoReportado = "LIGADO_TIMER_3S";
    // ---------------------

    } else {
        Serial.println("[LED] Comando desconhecido.");
        estadoReportado = "COMANDO_INVALIDO";
    }

    // Resposta JSON
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