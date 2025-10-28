#include "Ds18b20Sensor.h"
#include <Arduino.h>

// --- Implementação do Construtor ---
Ds18b20Sensor::Ds18b20Sensor(int pin, String topic_base, PubSubClient* mqttClient, unsigned long interval) {
    _pin = pin;
    _client = mqttClient;
    _interval = interval;
    _lastReadTime = 0; // Inicia em 0 para ler imediatamente
    _topic = topic_base + "/" + String(pin);

    // Aloca os objetos na memória
    _oneWireBus = new OneWire(_pin);
    _dallasSensors = new DallasTemperature(_oneWireBus);
}

// --- Implementação do Destrutor ---
Ds18b20Sensor::~Ds18b20Sensor() {
    // Libera a memória alocada
    delete _dallasSensors;
    delete _oneWireBus;
}

// --- Implementação do Setup ---
void Ds18b20Sensor::setup() {
    // Inicia a biblioteca DallasTemperature
    _dallasSensors->begin();
    Serial.printf("[DS18B20] Sensor inicializado no pino %d (1-Wire). Publicando em %s\n", _pin, _topic.c_str());
}

// --- Implementação do Loop ---
void Ds18b20Sensor::loop() {
    // Respeita o intervalo de leitura
    if (millis() - _lastReadTime >= _interval) {
        _lastReadTime = millis();

        // Envia o comando para todos os sensores no barramento lerem a temperatura
        _dallasSensors->requestTemperatures(); 
        
        // Pega a temperatura do *primeiro* sensor encontrado (índice 0)
        // A chamada requestTemperatures() é bloqueante por padrão,
        // então a leitura aqui já estará pronta.
        float tempC = _dallasSensors->getTempCByIndex(0);

        // Verifica se a leitura falhou (ex: sensor desconectado)
        if (tempC == DEVICE_DISCONNECTED_C) {
            Serial.printf("[DS18B20] Erro: Sensor no pino %d desconectado.\n", _pin);
            _client->publish(_topic.c_str(), "ERRO");
            return;
        }

        Serial.printf("[DS18B20] Pino %d - Temperatura: %.2f C\n", _pin, tempC);

        // Converte o float para uma string
        char payload[10];
        dtostrf(tempC, 4, 2, payload); // Formato: 4 chars total, 2 casas decimais
        
        _client->publish(_topic.c_str(), payload);
    }
}

// --- Implementação do getType ---
String Ds18b20Sensor::getType() {
    return "ds18b20";
}