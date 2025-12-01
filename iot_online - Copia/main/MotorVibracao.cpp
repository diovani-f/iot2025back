#include "MotorVibracao.h"
#include <Arduino.h>
#include <ArduinoJson.h>

// --- Construtor ---
MotorVibracao::MotorVibracao(int pin, String topic_base, PubSubClient* mqttClient) {
    _pin = pin;
    _client = mqttClient;
    _controlTopic = topic_base + "/" + String(pin);
    _autoOffTime = 0;
}

// --- Setup ---
void MotorVibracao::setup() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    Serial.printf("[MotorVibracao] Atuador inicializado no pino %d.\n", _pin);
    Serial.printf("[MotorVibracao] Ouvindo no tópico: %s\n", _controlTopic.c_str());
}

// --- Loop (Lógica do Timer) ---
void MotorVibracao::loop() {
    if (_autoOffTime > 0) {
        if (millis() > _autoOffTime) {
            digitalWrite(_pin, LOW);
            _autoOffTime = 0;
            
            Serial.printf("[MotorVibracao] Pino %d - Desligamento automatico (Timer).\n", _pin);

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
String MotorVibracao::getType() {
    return "motor_vibracao";
}

// --- Handle Message ---
void MotorVibracao::handleMqttMessage(String topic, String payload) {
    if (topic != _controlTopic) return;

    Serial.printf("[MotorVibracao] Pino %d - Comando recebido: %s\n", _pin, payload.c_str());

    String estadoReportado = "ERRO";

    if (payload == "ON") {
        digitalWrite(_pin, HIGH);
        _autoOffTime = 0;
        estadoReportado = "LIGADO";
        
    } else if (payload == "OFF") {
        digitalWrite(_pin, LOW);
        _autoOffTime = 0;
        estadoReportado = "DESLIGADO";

    // --- NOVOS COMANDOS ---
    } else if (payload == "ON_1S") {
        digitalWrite(_pin, HIGH);
        _autoOffTime = millis() + 1000;
        estadoReportado = "LIGADO_TIMER_1S";

    } else if (payload == "ON_3S") {
        digitalWrite(_pin, HIGH);
        _autoOffTime = millis() + 3000;
        estadoReportado = "LIGADO_TIMER_3S";
    // ---------------------

    } else {
        Serial.println("[MotorVibracao] Comando desconhecido.");
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