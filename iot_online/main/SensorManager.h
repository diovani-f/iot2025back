#pragma once
#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h> 

void sensorManagerSetup(PubSubClient* client, String deviceId);

void addSensor(JsonObject config);

void sensorManagerLoop();

void sensorManagerHandleMessage(String topic, String payload);