#pragma once
#include "Sensor.h"       
#include <PubSubClient.h> 

class Botao : public Sensor {
private:
    
    int _pin;
    String _topic;
    PubSubClient* _client; 
    int _estadoAnterior;

public:
    
    Botao(int pin, String topic_base, PubSubClient* mqttClient);

    
    void setup() override;
    void loop() override;
    String getType() override;
};