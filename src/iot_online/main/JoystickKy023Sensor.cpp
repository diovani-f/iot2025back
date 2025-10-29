#include "JoystickKy023Sensor.h"
#include <Arduino.h>
#include <ArduinoJson.h> // Para publicar X/Y

// --- Implementação do Construtor ---
JoystickKy023Sensor::JoystickKy023Sensor(byte* pins, String topic_base, PubSubClient* mqttClient, unsigned long interval) {
    _pins = pins; // Salva o ponteiro para o array de pinos
    _client = mqttClient;
    _interval = interval;
    _lastReadTime = 0;
    
    // Cria um tópico único (ex: baseado no pino SW)
    _baseTopic = topic_base + "/sw" + String(_pins[2]);
}

// --- Implementação do Destrutor ---
JoystickKy023Sensor::~JoystickKy023Sensor() {
    delete[] _pins; // Libera a memória do array de pinos
}

// --- Implementação do Setup ---
void JoystickKy023Sensor::setup() {
    // Configura os pinos analógicos (X e Y)
    pinMode(_pins[0], INPUT); // X_PIN
    pinMode(_pins[1], INPUT); // Y_PIN

    // Configura o pino digital (SW) com pull-up interno
    pinMode(_pins[2], INPUT_PULLUP); // SW_PIN
    _lastSwState = digitalRead(_pins[2]);

    Serial.printf("[Joystick] Sensor inicializado. X:%d, Y:%d, SW:%d. Publicando em %s/...\n", _pins[0], _pins[1], _pins[2], _baseTopic.c_str());
}

// --- Implementação do Loop ---
void JoystickKy023Sensor::loop() {
    
    // --- 1. Checagem do Botão (Event-driven) ---
    // Roda a cada loop para não perder cliques
    int swState = digitalRead(_pins[2]);
    if (_lastSwState == HIGH && swState == LOW) {
        Serial.printf("[Joystick] SW %d - Pressionado!\n", _pins[2]);
        String swTopic = _baseTopic + "/switch";
        if (_client->connected()) {
            _client->publish(swTopic.c_str(), "pressionado");
        }
        delay(50); // Debounce
    }
    _lastSwState = swState;

    // --- 2. Checagem dos Eixos X/Y (Time-driven) ---
    // Roda apenas no intervalo de tempo definido
    if (millis() - _lastReadTime >= _interval) {
        _lastReadTime = millis();

        int xVal = analogRead(_pins[0]);
        int yVal = analogRead(_pins[1]);

        // Cria o JSON com os valores
        DynamicJsonDocument doc(128);
        doc["x"] = xVal;
        doc["y"] = yVal;

        char payload[128];
        serializeJson(doc, payload, sizeof(payload));

        Serial.printf("[Joystick] Pinos %d,%d - Publicando: %s\n", _pins[0], _pins[1], payload);
        String posTopic = _baseTopic + "/position";
        if (_client->connected()) {
            _client->publish(posTopic.c_str(), payload);
        }
    }
}

// --- Implementação do getType ---
String JoystickKy023Sensor::getType() {
    return "joystick_ky023";
}