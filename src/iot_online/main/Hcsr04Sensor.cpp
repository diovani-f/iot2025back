#include "Hcsr04Sensor.h"
#include <Arduino.h>


const unsigned long MAX_ECHO_TIME_US = 30000;

Hcsr04Sensor::Hcsr04Sensor(int trigPin, int echoPin, String topic_base, PubSubClient* mqttClient, unsigned long interval) {
    _trigPin = trigPin;
    _echoPin = echoPin;
    _client = mqttClient;
    _interval = interval;
    _lastReadTime = 0; // Inicia em 0 para ler imediatamente

    // Cria um tópico único baseado no pino de Trigger
    _topic = topic_base + "/trig" + String(trigPin); 
}

// --- Implementação do Setup ---
void Hcsr04Sensor::setup() {
    pinMode(_trigPin, OUTPUT);
    pinMode(_echoPin, INPUT);
    digitalWrite(_trigPin, LOW); // Garante que o trigger comece desligado
    Serial.printf("[HC-SR04] Sensor inicializado. Trig: %d, Echo: %d. Publicando em %s\n", _trigPin, _echoPin, _topic.c_str());
}

// --- Implementação do Loop ---
void Hcsr04Sensor::loop() {
    // Este sensor, assim como o encoder, não lê a cada loop.
    // Ele respeita um intervalo para não sobrecarregar o sistema.
    if (millis() - _lastReadTime >= _interval) {
        _lastReadTime = millis(); // Reseta o timer

        float distance = readDistance(); // Chama a função de medição

        if (distance > 0) {
            // Leitura válida
            Serial.printf("[HC-SR04] Trig %d - Distancia: %.2f cm\n", _trigPin, distance);
            
            // Converte o float para uma string
            char payload[10];
            dtostrf(distance, 4, 2, payload); // Formato: 4 chars total, 2 casas decimais
            
            _client->publish(_topic.c_str(), payload);
        } else {
            // Leitura inválida (fora de alcance / timeout)
            Serial.printf("[HC-SR04] Trig %d - Fora de alcance.\n", _trigPin);
            _client->publish(_topic.c_str(), "0.00");
        }
    }
}

// --- Implementação do getType ---
String Hcsr04Sensor::getType() {
    return "hcsr04";
}


// --- Função Privada de Medição ---
float Hcsr04Sensor::readDistance() {
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigPin, LOW);


    long duration_us = pulseIn(_echoPin, HIGH, MAX_ECHO_TIME_US);

    if (duration_us == 0) {
        return 0.0;
    }

    float distance_cm = (duration_us * 0.0343) / 2.0;

    return distance_cm;
}