#pragma once
#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h> 

// Inicializa o gerenciador (AGORA COM DEVICE ID)
void sensorManagerSetup(PubSubClient* client, String deviceId);

// Função "Factory" que cria e adiciona um sensor a partir da sua configuração JSON
void addSensor(JsonObject config);

// Loop principal que itera por todos os sensores
void sensorManagerLoop();

// Repassa uma mensagem MQTT para todos os objetos gerenciados
void sensorManagerHandleMessage(String topic, String payload);