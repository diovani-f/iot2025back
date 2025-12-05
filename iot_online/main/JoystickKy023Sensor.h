#pragma once
#include "Sensor.h"
#include <PubSubClient.h>

class JoystickKy023Sensor : public Sensor {
private:
    byte* _pins; 
    
    String _baseTopic;
    PubSubClient* _client;

    unsigned long _lastReadTime;
    unsigned long _interval; 
    int _lastSwState; 

public:
    
    
    JoystickKy023Sensor(byte* pins, String topic_base, PubSubClient* mqttClient, unsigned long interval = 250);
    
    
    ~JoystickKy023Sensor(); 

    
    void setup() override;
    void loop() override;
    String getType() override;
};