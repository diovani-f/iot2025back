#pragma once
#include "Sensor.h"
#include <PubSubClient.h>


#include <Adafruit_APDS9960.h>
#include <Wire.h> 

class Apds9960Sensor : public Sensor {
private:
    int _sdaPin;
    int _sclPin;
    String _baseTopic; 
    PubSubClient* _client;

    unsigned long _lastReadTime;
    unsigned long _interval; 

    
    TwoWire* _i2c_bus;
    
    
    Adafruit_APDS9960 _apds;

public:
    
    
    Apds9960Sensor(int sdaPin, int sclPin, String topic_base, PubSubClient* mqttClient, unsigned long interval = 500);
    
    
    ~Apds9960Sensor();

    
    void setup() override;
    void loop() override;
    String getType() override;
};