#include "Apds9960Sensor.h"
#include <Arduino.h>
#include <ArduinoJson.h> // Para publicar a Cor como JSON

// --- Implementação do Construtor ---
// CORREÇÃO: Removida a inicialização redundante de _apds()
Apds9960Sensor::Apds9960Sensor(int sdaPin, int sclPin, String topic_base, PubSubClient* mqttClient, unsigned long interval) 
    : _i2c_bus(new TwoWire(1)) {
    
    _sdaPin = sdaPin;
    _sclPin = sclPin;
    _client = mqttClient;
    _interval = interval;
    _lastReadTime = 0;
    
    // Tópico único baseado no pino SDA
    _baseTopic = topic_base + "/sda" + String(sdaPin);
}

// --- Implementação do Destrutor ---
Apds9960Sensor::~Apds9960Sensor() {
    delete _i2c_bus; // Libera a memória alocada para o barramento I2C
}

// --- Implementação do Setup ---
void Apds9960Sensor::setup() {
    // 1. Inicia o barramento I2C nos pinos especificados
    _i2c_bus->begin(_sdaPin, _sclPin);

    // 2. Inicia o sensor APDS-9960 neste barramento
    // CORREÇÃO: A biblioteca espera o endereço (0x39) e o ponteiro do barramento
    if (!_apds.begin(10, APDS9960_AGAIN_4X, 0x39, _i2c_bus)) { // <-- VERSÃO CORRIGIDA
        Serial.printf("[APDS-9960] Erro ao inicializar sensor nos pinos SDA:%d, SCL:%d\n", _sdaPin, _sclPin);
        return;
    }

    // 3. Habilita todas as funções do sensor
    _apds.enableProximity(true);
    _apds.enableGesture(true);
    _apds.enableColor(true); 

    Serial.printf("[APDS-9960] Sensor inicializado. SDA:%d, SCL:%d. Publicando em %s/...\n", _sdaPin, _sclPin, _baseTopic.c_str());
}

// --- Implementação do Loop ---
void Apds9960Sensor::loop() {
    
    // --- 1. Checagem de Gestos (Event-driven) ---
    uint8_t gesture = _apds.readGesture();
    String gestureStr = "";
    
    if (gesture == APDS9960_UP)    gestureStr = "UP";
    if (gesture == APDS9960_DOWN)  gestureStr = "DOWN";
    if (gesture == APDS9960_LEFT)  gestureStr = "LEFT";
    if (gesture == APDS9960_RIGHT) gestureStr = "RIGHT";

    if (gestureStr != "") {
        Serial.printf("[APDS-9960] SDA %d - Gesto: %s\n", _sdaPin, gestureStr.c_str());
        String gestureTopic = _baseTopic + "/gesture";
        _client->publish(gestureTopic.c_str(), gestureStr.c_str());
    }

    // --- 2. Checagem de Proximidade/Cor/Luz (Time-driven) ---
    if (millis() - _lastReadTime >= _interval) {
        _lastReadTime = millis();

        // Leitura de Proximidade
        uint8_t prox = _apds.readProximity();
        Serial.printf("[APDS-9960] SDA %d - Proximidade: %d\n", _sdaPin, prox);
        String proxTopic = _baseTopic + "/proximity";
        char proxPayload[5];
        itoa(prox, proxPayload, 10);
        _client->publish(proxTopic.c_str(), proxPayload);

        // Leitura de Cor (R, G, B) e Luz Ambiente (C - Clear)
        uint16_t r, g, b, c;
        _apds.getColorData(&r, &g, &b, &c);

        // Publica a Luz Ambiente
        Serial.printf("[APDS-9960] SDA %d - Luz Ambiente: %d\n", _sdaPin, c);
        String ambientTopic = _baseTopic + "/ambient_light";
        char ambientPayload[7];
        itoa(c, ambientPayload, 10);
        _client->publish(ambientTopic.c_str(), ambientPayload);

        // Publica a Cor como um JSON
        DynamicJsonDocument doc(128);
        doc["r"] = r;
        doc["g"] = g;
        doc["b"] = b;
        
        char colorPayload[64];
        serializeJson(doc, colorPayload, sizeof(colorPayload));
        Serial.printf("[APDS-9960] SDA %d - Cor: %s\n", _sdaPin, colorPayload);
        String colorTopic = _baseTopic + "/color";
        
        // CORREÇÃO: 'c_c_str()' corrigido para 'c_str()'
        _client->publish(colorTopic.c_str(), colorPayload);
    }
}

// --- Implementação do getType ---
String Apds9960Sensor::getType() {
    return "apds9960";
}