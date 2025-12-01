#include "EncoderSensor.h"
#include <Arduino.h>
#include <ArduinoJson.h>

// --- Implementação do Construtor ---
EncoderSensor::EncoderSensor(int pin, String topic_base, PubSubClient* mqttClient, unsigned long interval) {
    _pin = pin;
    _client = mqttClient;
    _topic = topic_base + "/" + String(pin);
    _interval = interval;
    _pulseCount = 0; // Não utilizado neste modo, mas mantido para compatibilidade
    _lastCalcTime = 0;
}

// --- Implementação do Setup ---
void EncoderSensor::setup() {
    // Configura o pino como entrada com resistor de PULL-UP interno.
    // Isso garante que, quando o sensor estiver aberto (sem sinal), ele leia 1 (HIGH).
    // Quando o sensor fechar contato com o GND, ele lerá 0 (LOW).
    pinMode(_pin, INPUT_PULLUP); 
    
    Serial.printf("[Encoder] Sensor de Estado (Real) iniciado no pino %d. Publicando em %s\n", _pin, _topic.c_str());
}

// --- Implementação do Loop ---
void EncoderSensor::loop() {
    // Verifica se já passou o tempo do intervalo (ex: 1 segundo)
    if (millis() - _lastCalcTime >= _interval) {
        _lastCalcTime = millis();
        
        // --- LEITURA REAL DO HARDWARE ---
        // Lê o estado atual do pino: 
        // 1 (HIGH) = Geralmente Aberto
        // 0 (LOW)  = Geralmente Fechado/Obstruído
        int estadoAtual = digitalRead(_pin);

        Serial.printf("[Encoder] Pino %d - Estado Lógico: %d\n", _pin, estadoAtual);

        // --- MONTAGEM DO JSON ---
        DynamicJsonDocument doc(128);
        doc["status"] = "OK";
        
        // Envia o valor booleano (0 ou 1)
        doc["aberto"] = estadoAtual; 

        // 2. Serializa o JSON
        char payload[128];
        serializeJson(doc, payload, sizeof(payload));

        // 3. Publica o JSON com verificação de segurança
        if (_client->connected()) {
            _client->publish(_topic.c_str(), payload);
        } else {
            Serial.println("[Encoder] Erro: MQTT desconectado. Mensagem não enviada.");
        }
    }
}

// --- Implementação do getType ---
String EncoderSensor::getType() {
    return "encoder";
}

// --- Funções de Interrupção (Não usadas neste modo de leitura direta) ---
void IRAM_ATTR EncoderSensor::isr_wrapper(void* arg) {}
void EncoderSensor::handleInterrupt() {}