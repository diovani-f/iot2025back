#pragma once
#include "Sensor.h"
#include <PubSubClient.h>

class LedAtuador : public Sensor {
private:
    int _pin;
    String _controlTopic;
    PubSubClient* _client;
    
    
    unsigned long _autoOffTime; 

public:
    LedAtuador(int pin, String topic_base, PubSubClient* mqttClient);
    
    void setup() override;
    void loop() override; 
    String getType() override;
    
    void handleMqttMessage(String topic, String payload) override;
};