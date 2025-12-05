#pragma once
#include "Sensor.h"
#include <PubSubClient.h>
#include <Keypad.h> 

class KeypadSensor : public Sensor {
private:
    byte* _rowPins; 
    byte* _colPins; 
    
    char _keys[4][4] = {
      {'1','2','3','A'},
      {'4','5','6','B'},
      {'7','8','9','C'},
      {'*','0','#','D'}
    };

    Keypad* _keypad; 
    
    String _topic;
    PubSubClient* _client;

    String _bufferSenha; 

public:
    KeypadSensor(byte* rowPins, byte* colPins, String topic_base, PubSubClient* mqttClient);
    ~KeypadSensor(); 

    void setup() override;
    void loop() override;
    String getType() override;
};