#include "MotorVibracao.h"
#include <Arduino.h>

// --- Implementação do Construtor ---
MotorVibracao::MotorVibracao(int pin, String topic_base, PubSubClient* mqttClient) {
    _pin = pin;
    _client = mqttClient;
    
    // Cria o tópico de controle que este motor vai escutar
    // Ex: grupoX/atuador/vibracao/15
    _controlTopic = topic_base + "/" + String(pin);
}

// --- Implementação do Setup ---
void MotorVibracao::setup() {
    // Configura o pino como SAÍDA
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW); // Garante que comece desligado
    
    Serial.printf("[MotorVibracao] Atuador inicializado no pino %d.\n", _pin);
    Serial.printf("[MotorVibracao] Ouvindo no tópico: %s\n", _controlTopic.c_str());
}

// --- Implementação do Loop ---
void MotorVibracao::loop() {
    // Este atuador é "reativo". O loop não faz nada.
    // A lógica está toda no 'handleMqttMessage'.
}

// --- Implementação do getType ---
String MotorVibracao::getType() {
    return "motor_vibracao";
}

// --- Implementação do HandleMessage ---
void MotorVibracao::handleMqttMessage(String topic, String payload) {
    // 1. Verifica se a mensagem é para ESTE motor específico
    if (topic != _controlTopic) {
        return; // Não é para mim, ignoro a mensagem.
    }

    // 2. Processa o comando
    Serial.printf("[MotorVibracao] Pino %d - Comando recebido: %s\n", _pin, payload.c_str());

    if (payload == "ON") {
        digitalWrite(_pin, HIGH);
    } else if (payload == "OFF") {
        digitalWrite(_pin, LOW);
    } else {
        Serial.println("[MotorVibracao] Comando desconhecido. Use 'ON' ou 'OFF'.");
    }
}