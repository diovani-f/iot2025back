#include "DhtSensor.h"
#include <Arduino.h>
#include <ArduinoJson.h>

DhtSensor::DhtSensor(int pin, uint8_t dht_type, String typeString, String topic_base, PubSubClient* mqttClient, unsigned long interval) {
    _pin = pin;
    _dht_type = dht_type;
    _typeString = typeString;
    _client = mqttClient;
    _interval = interval;
    _lastReadTime = 0 - interval; 
    _topic = topic_base + "/" + String(pin);
    _dht = new DHT(_pin, _dht_type);
}

DhtSensor::~DhtSensor() {
    delete _dht;
}

void DhtSensor::setup() {
    _dht->begin();
    Serial.printf("[DHT] Sensor %s inicializado no pino %d. Publicando em %s\n", _typeString.c_str(), _pin, _topic.c_str());
}

void DhtSensor::loop() {
    if (millis() - _lastReadTime >= _interval) {
        _lastReadTime = millis();

        float h = _dht->readHumidity();
        float t = _dht->readTemperature();

        DynamicJsonDocument doc(128);
        char payload[128];

        if (isnan(h) || isnan(t)) {
            Serial.printf("[DHT] Falha ao ler o sensor %s no pino %d!\n", _typeString.c_str(), _pin);
            doc["status"] = "ERRO";
            doc["erro"] = "Falha na leitura";
        } else {
            Serial.printf("[DHT] %s (Pino %d) - T:%.2f C, U:%.2f %%\n", _typeString.c_str(), _pin, t, h);
            doc["status"] = "OK";
            doc["temperatura_c"] = t;
            doc["umidade_pct"] = h;
        }

        serializeJson(doc, payload, sizeof(payload));

        if (_client->connected()) {
            _client->publish(_topic.c_str(), payload);
        } else {
            Serial.println("[DHT] Erro: MQTT desconectado. Mensagem não enviada.");
        }
    }
}

String DhtSensor::getType() {
    return _typeString;
}