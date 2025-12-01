#include "KeypadSensor.h"
#include <Arduino.h>
#include <ArduinoJson.h> 

// --- Construtor ---
KeypadSensor::KeypadSensor(byte* rowPins, byte* colPins, String topic_base, PubSubClient* mqttClient) {
    _rowPins = rowPins; 
    _colPins = colPins; 
    _client = mqttClient;
    _topic = topic_base + "/row" + String(rowPins[0]);
    _keypad = new Keypad(makeKeymap(_keys), _rowPins, _colPins, 4, 4);
    
    _bufferSenha = ""; // Inicializa o buffer vazio
}

// --- Destrutor ---
KeypadSensor::~KeypadSensor() {
    delete _keypad;   
    delete[] _rowPins;
    delete[] _colPins;
}

// --- Setup ---
void KeypadSensor::setup() {
    Serial.printf("[Keypad] Sensor (Buffer) inicializado. Publicando em %s\n", _topic.c_str());
}

// --- Loop (Com Lógica de Buffer) ---
void KeypadSensor::loop() {
    char key = _keypad->getKey();

    if (key) { 
        Serial.printf("[Keypad] Tecla física: %c\n", key);

        // 1. Se apertar '*', limpa tudo e começa do zero
        if (key == '*') {
            _bufferSenha = "*"; 
            Serial.println("[Keypad] Início de senha detectado (*). Buffer limpo.");
            return; // Não envia nada ainda
        }

        // 2. Se o buffer começar com '*', começamos a acumular
        if (_bufferSenha.startsWith("*")) {
            _bufferSenha += key; // Adiciona a tecla ao buffer (ex: "*1", "*12"...)
            
            Serial.println("[Keypad] Digitando: " + _bufferSenha);

            // 3. Verifica se completou o tamanho (* + 4 digitos = 5 caracteres)
            if (_bufferSenha.length() == 5) {
                
                Serial.println("[Keypad] Senha completa! Enviando...");

                // --- MONTA O JSON DA SENHA COMPLETA ---
                DynamicJsonDocument doc(128);
                doc["status"] = "OK";
                doc["senha_completa"] = _bufferSenha; // Envia "*1234"

                char payload[128];
                serializeJson(doc, payload, sizeof(payload));

                // Publica no MQTT
                if (_client->connected()) {
                    _client->publish(_topic.c_str(), payload);
                } else {
                    Serial.println("[Keypad] Erro: MQTT desconectado.");
                }

                // Limpa o buffer para a próxima tentativa
                _bufferSenha = ""; 
            }
        } 
        // Se apertar tecla sem ter apertado '*' antes, ignora ou trata como erro (aqui estamos ignorando)
    }
}

// --- getType ---
String KeypadSensor::getType() {
    return "keypad4x4";
}