#pragma once
#include "Sensor.h"
#include <PubSubClient.h>

class IrReceiverSensor : public Sensor {
private:
    int _pin;
    String _topic;
    PubSubClient* _client;

public:
    
    IrReceiverSensor(int pin, String topic_base, PubSubClient* mqttClient);
    
    
    ~IrReceiverSensor(); 

    
    void setup() override;
    void loop() override;
    String getType() override;
};