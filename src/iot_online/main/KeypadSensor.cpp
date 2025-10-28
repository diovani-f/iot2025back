#include "KeypadSensor.h"
#include <Arduino.h>

// --- Implementação do Construtor ---
KeypadSensor::KeypadSensor(byte* rowPins, byte* colPins, String topic_base, PubSubClient* mqttClient) {
    _rowPins = rowPins; // Salva o ponteiro
    _colPins = colPins; // Salva o ponteiro
    _client = mqttClient;
    
    // Cria um tópico único (ex: baseado no primeiro pino de linha)
    _topic = topic_base + "/row" + String(rowPins[0]);
    
    // Cria o objeto Keypad
    _keypad = new Keypad(makeKeymap(_keys), _rowPins, _colPins, 4, 4);
}

// --- Implementação do Destrutor ---
KeypadSensor::~KeypadSensor() {
    delete _keypad;    // Deleta o objeto keypad
    delete[] _rowPins; // Libera a memória do array de pinos de linha
    delete[] _colPins; // Libera a memória do array de pinos de coluna
}

// --- Implementação do Setup ---
void KeypadSensor::setup() {
    // A biblioteca Keypad cuida da configuração dos pinos no construtor
    Serial.printf("[Keypad] Sensor inicializado. Publicando em %s\n", _topic.c_str());
}

// --- Implementação do Loop ---
void KeypadSensor::loop() {
    // getKey() é não-bloqueante
    char key = _keypad->getKey();

    if (key) { // Se uma tecla foi pressionada
        Serial.printf("[Keypad] Tecla pressionada: %c\n", key);

        // Prepara um payload de 2 caracteres (a tecla + o terminador nulo)
        char payload[2] = {key, '\0'}; 
        
        _client->publish(_topic.c_str(), payload);
    }
}

// --- Implementação do getType ---
String KeypadSensor::getType() {
    return "keypad4x4";
}