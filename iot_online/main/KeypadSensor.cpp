#include "KeypadSensor.h"
#include <Arduino.h>
#include <ArduinoJson.h> 


KeypadSensor::KeypadSensor(byte* rowPins, byte* colPins, String topic_base, PubSubClient* mqttClient) {
    _rowPins = rowPins; 
    _colPins = colPins; 
    _client = mqttClient;
    _topic = topic_base + "/row" + String(rowPins[0]);
    _keypad = new Keypad(makeKeymap(_keys), _rowPins, _colPins, 4, 4);
    
    _bufferSenha = ""; 
}


KeypadSensor::~KeypadSensor() {
    delete _keypad;   
    delete[] _rowPins;
    delete[] _colPins;
}


void KeypadSensor::setup() {
    Serial.printf("[Keypad] Sensor (Buffer) inicializado. Publicando em %s\n", _topic.c_str());
}


void KeypadSensor::loop() {
    char key = _keypad->getKey();

    if (key) { 
        Serial.printf("[Keypad] Tecla física: %c\n", key);

        
        if (key == '*') {
            _bufferSenha = "*"; 
            Serial.println("[Keypad] Início de senha detectado (*). Buffer limpo.");
            return; 
        }

        
        if (_bufferSenha.startsWith("*")) {
            _bufferSenha += key; 
            
            Serial.println("[Keypad] Digitando: " + _bufferSenha);

            
            if (_bufferSenha.length() == 5) {
                
                Serial.println("[Keypad] Senha completa! Enviando...");

                
                DynamicJsonDocument doc(128);
                doc["status"] = "OK";
                doc["senha_completa"] = _bufferSenha; 

                char payload[128];
                serializeJson(doc, payload, sizeof(payload));

                
                if (_client->connected()) {
                    _client->publish(_topic.c_str(), payload);
                } else {
                    Serial.println("[Keypad] Erro: MQTT desconectado.");
                }

                
                _bufferSenha = ""; 
            }
        } 
        
    }
}


String KeypadSensor::getType() {
    return "keypad4x4";
}