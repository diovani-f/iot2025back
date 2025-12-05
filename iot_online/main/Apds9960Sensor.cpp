#include "Apds9960Sensor.h"
#include <Arduino.h>
#include <ArduinoJson.h>


Apds9960Sensor::Apds9960Sensor(int sdaPin, int sclPin, String topic_base, PubSubClient* mqttClient, unsigned long interval) 
    : _i2c_bus(new TwoWire(1)) {
    
    _sdaPin = sdaPin;
    _sclPin = sclPin;
    _client = mqttClient;
    _interval = interval;
    _lastReadTime = 0;
    _baseTopic = topic_base + "/sda" + String(sdaPin);
}


Apds9960Sensor::~Apds9960Sensor() {
    delete _i2c_bus;
}


void Apds9960Sensor::setup() {
    _i2c_bus->begin(_sdaPin, _sclPin);

    
    if (!_apds.begin(10, APDS9960_AGAIN_4X, 0x39, _i2c_bus)) {
        Serial.printf("[APDS-9960] Erro ao inicializar sensor nos pinos SDA:%d, SCL:%d\n", _sdaPin, _sclPin);
        return;
    }

    _apds.enableProximity(true);
    _apds.enableGesture(true);
    _apds.enableColor(true); 

    Serial.printf("[APDS-9960] Sensor inicializado. SDA:%d, SCL:%d. Publicando em %s/...\n", _sdaPin, _sclPin, _baseTopic.c_str());
}


void Apds9960Sensor::loop() {
    
    
    uint8_t gesture = _apds.readGesture();
    String gestureStr = "";
    
    if (gesture == APDS9960_UP)    gestureStr = "UP";
    if (gesture == APDS9960_DOWN)  gestureStr = "DOWN";
    if (gesture == APDS9960_LEFT)  gestureStr = "LEFT";
    if (gesture == APDS9960_RIGHT) gestureStr = "RIGHT";

    if (gestureStr != "") {
        Serial.printf("[APDS-9960] SDA %d - Gesto: %s\n", _sdaPin, gestureStr.c_str());
        
        
        DynamicJsonDocument doc(64);
        doc["status"] = "OK";
        doc["gesto"] = gestureStr;
        char payload[64];
        serializeJson(doc, payload, sizeof(payload));

        String gestureTopic = _baseTopic + "/gesture";
        
        if (_client->connected()) {
            _client->publish(gestureTopic.c_str(), payload);
        }
    }

    
    if (millis() - _lastReadTime >= _interval) {
        _lastReadTime = millis();

        
        uint8_t prox = _apds.readProximity();
        uint16_t r, g, b, c;
        _apds.getColorData(&r, &g, &b, &c);

        DynamicJsonDocument doc(256);
        doc["status"] = "OK";
        doc["proximidade"] = prox;
        doc["luz_ambiente"] = c;
        
        
        JsonObject cor = doc.createNestedObject("cor");
        cor["r"] = r;
        cor["g"] = g;
        cor["b"] = b;
        
        
        char payload[256];
        serializeJson(doc, payload, sizeof(payload));

        Serial.printf("[APDS-9960] SDA %d - Publicando dados: %s\n", _sdaPin, payload);
        
        
        String dataTopic = _baseTopic + "/data";
        if (_client->connected()) {
            _client->publish(dataTopic.c_str(), payload);
        }
        
    }
}


String Apds9960Sensor::getType() {
    return "apds9960";
}