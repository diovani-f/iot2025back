#include "DhtSensor.h"
#include <Arduino.h>
#include <ArduinoJson.h> // Para criar o JSON de resposta

// --- Implementação do Construtor ---
DhtSensor::DhtSensor(int pin, uint8_t dht_type, String typeString, String topic_base, PubSubClient* mqttClient, unsigned long interval) {
    _pin = pin;
    _dht_type = dht_type;
    _typeString = typeString;
    _client = mqttClient;
    _interval = interval;
    _lastReadTime = 0 - interval; // Força uma leitura na primeira vez
    
    // Tópico único baseado no pino
    _topic = topic_base + "/p" + String(pin);

    // Cria o objeto DHT dinamicamente
    _dht = new DHT(_pin, _dht_type);
}

// --- Implementação do Destrutor ---
DhtSensor::~DhtSensor() {
    delete _dht; // Libera a memória do objeto DHT
}

// --- Implementação do Setup ---
void DhtSensor::setup() {
    _dht->begin(); // Inicializa o sensor
    Serial.printf("[DHT] Sensor %s inicializado no pino %d. Publicando em %s\n", _typeString.c_str(), _pin, _topic.c_str());
}

// --- Implementação do Loop ---
void DhtSensor::loop() {
    // A biblioteca DHT exige um intervalo de pelo menos 2 segundos
    if (millis() - _lastReadTime >= _interval) {
        _lastReadTime = millis();

        // Faz a leitura dos dois valores
        float h = _dht->readHumidity();
        // Lê a temperatura em Celsius (é o padrão)
        float t = _dht->readTemperature();

        // Verifica se a leitura falhou (retorna NaN)
        if (isnan(h) || isnan(t)) {
            Serial.printf("[DHT] Falha ao ler o sensor %s no pino %d!\n", _typeString.c_str(), _pin);
            return;
        }

        // --- Cria o Payload JSON ---
        DynamicJsonDocument doc(128);
        doc["temperatura_c"] = t;
        doc["umidade_pct"] = h;

        // --- Publica o JSON ---
        char payload[128];
        serializeJson(doc, payload, sizeof(payload));

        Serial.printf("[DHT] %s (Pino %d) - Publicando JSON: %s\n", _typeString.c_str(), _pin, payload);
        _client->publish(_topic.c_str(), payload);
    }
}

// --- Implementação do getType ---
String DhtSensor::getType() {
    return _typeString; // Retorna "dht11" ou "dht22"
}