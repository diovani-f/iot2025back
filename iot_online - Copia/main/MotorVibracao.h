#pragma once
#include "Sensor.h" 
#include <PubSubClient.h>

class MotorVibracao : public Sensor {
private:
    int _pin;
    String _controlTopic; 
    PubSubClient* _client;
    
    // --- NOVO: Variável para o temporizador ---
    unsigned long _autoOffTime; 

public:
    MotorVibracao(int pin, String topic_base, PubSubClient* mqttClient);
    
    void setup() override;
    void loop() override; 
    String getType() override;
    
    void handleMqttMessage(String topic, String payload) override;
};