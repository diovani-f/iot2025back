#pragma once
#include "Sensor.h"
#include <PubSubClient.h>

class LedAtuador : public Sensor {
private:
    int _pin;
    String _controlTopic;
    PubSubClient* _client;
    
    // --- NOVO: Variável para controlar o tempo de desligamento ---
    unsigned long _autoOffTime; 

public:
    LedAtuador(int pin, String topic_base, PubSubClient* mqttClient);
    
    void setup() override;
    void loop() override; // Agora o loop será usado!
    String getType() override;
    
    void handleMqttMessage(String topic, String payload) override;
};