#include "EncoderSensor.h"
#include <Arduino.h> // Para Serial, pinMode, attachInterruptArg, etc.

// --- Implementação do Construtor ---
EncoderSensor::EncoderSensor(int pin, String topic_base, PubSubClient* mqttClient, unsigned long interval) {
    _pin = pin;
    _client = mqttClient;
    _topic = topic_base + "/" + String(pin);
    _interval = interval;
    _pulseCount = 0;
    _lastCalcTime = 0;
}

// --- Implementação do Setup ---
void EncoderSensor::setup() {
    // Configura o pino como entrada com pull-up interno
    pinMode(_pin, INPUT_PULLUP); 

    // Anexa a interrupção
    // Usamos attachInterruptArg para poder passar 'this' (o ponteiro para este objeto)
    // A interrupção será disparada na "borda de subida" (RISING)
    attachInterruptArg(digitalPinToInterrupt(_pin), isr_wrapper, this, RISING);

    Serial.printf("[Encoder] Sensor inicializado no pino %d. Publicando em %s\n", _pin, _topic.c_str());
}

// --- Implementação do Loop ---
void EncoderSensor::loop() {
    // Verifica se já passou o intervalo de tempo (ex: 1 segundo)
    if (millis() - _lastCalcTime >= _interval) {
        _lastCalcTime = millis(); // Reseta o timer

        unsigned long pulseCountCopy;

        // --- Seção Crítica ---
        // Desabilitamos as interrupções brevemente para copiar o valor
        // e zerar o contador com segurança.
        noInterrupts(); // Desabilita interrupções
        pulseCountCopy = _pulseCount; // Copia a contagem
        _pulseCount = 0;              // Zera o contador
        interrupts();   // Habilita interrupções
        // --- Fim da Seção Crítica ---

        // Calcula a velocidade em Pulsos Por Segundo (PPS)
        float pps = (float)pulseCountCopy / (_interval / 1000.0);

        Serial.printf("[Encoder] Pino %d - Pulsos: %lu, PPS: %.2f\n", _pin, pulseCountCopy, pps);

        // Converte o float para uma string para publicar
        char payload[10];
        dtostrf(pps, 4, 2, payload); // Converte float (pps) para string (payload)
        
        // Publica no tópico MQTT
        _client->publish(_topic.c_str(), payload);
    }
}

// --- Implementação do getType ---
String EncoderSensor::getType() {
    return "encoder";
}


// --- Implementação das Funções de Interrupção ---

// Esta é a função "wrapper" estática que a CPU chama
void IRAM_ATTR EncoderSensor::isr_wrapper(void* arg) {
    // Converte o argumento 'void*' de volta para um ponteiro do nosso objeto
    EncoderSensor* instance = static_cast<EncoderSensor*>(arg);
    // Chama o método de interrupção real do objeto
    instance->handleInterrupt();
}

// Este é o método que realmente faz o trabalho
void EncoderSensor::handleInterrupt() {
    // Apenas incrementa o contador.
    // Mantenha o código da ISR o mais rápido e curto possível.
    _pulseCount++;
}