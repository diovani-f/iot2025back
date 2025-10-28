#include "IrReceiverSensor.h"
#include <Arduino.h>
#include <IRremote.h>

// --- Implementação do Construtor ---
IrReceiverSensor::IrReceiverSensor(int pin, String topic_base, PubSubClient* mqttClient) {
    _pin = pin;
    _client = mqttClient;
    _topic = topic_base + "/" + String(pin); 
}

// --- Implementação do Destrutor ---
IrReceiverSensor::~IrReceiverSensor() {
    // Para o receptor global da biblioteca IR
    IrReceiver.stop();
    Serial.printf("[IR Receiver] Sensor (global) parado no pino %d.\n", _pin);
}

// --- Implementação do Setup ---
void IrReceiverSensor::setup() {
    // Inicia o receptor IR global da biblioteca no pino especificado
    // ENABLE_LED_FEEDBACK faz o LED_BUILTIN piscar ao receber (útil para debug)
    IrReceiver.begin(_pin, ENABLE_LED_FEEDBACK);
    Serial.printf("[IR Receiver] Sensor (global) iniciado no pino %d. Publicando em %s\n", _pin, _topic.c_str());
}

// --- Implementação do Loop ---
void IrReceiverSensor::loop() {
    // Verifica se o receptor global tem um novo código decodificado
    if (IrReceiver.decode()) {
        
        // Pega o valor RAW (completo) decodificado.
        // Usamos decodedRawData para pegar o código, mesmo de protocolos desconhecidos
        unsigned long hexValue = IrReceiver.decodedIRData.decodedRawData;

        // A biblioteca retorna 0 para repetições ou códigos inválidos, então filtramos
        if (hexValue != 0) {
            Serial.printf("[IR Receiver] Pino %d - Código recebido: 0x%lX\n", _pin, hexValue);

            // Converte o valor HEX (long) para uma String
            // Um buffer de 12 é seguro ("0x" + 8 chars hex + nulo)
            char payload[12];
            sprintf(payload, "0x%lX", hexValue);

            // Publica no tópico MQTT
            _client->publish(_topic.c_str(), payload);
        }

        // Continua ouvindo o próximo sinal
        IrReceiver.resume(); 
    }
}

// --- Implementação do getType ---
String IrReceiverSensor::getType() {
    return "ir_receiver";
}