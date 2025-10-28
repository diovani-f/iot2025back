#include "Mpu6050Sensor.h"
#include <Arduino.h>
#include <ArduinoJson.h> // <--- Usaremos para CRIAR JSON

// --- Implementação do Construtor ---
// Usamos o barramento I2C '0' (o ESP32 tem 0 e 1)
Mpu6050Sensor::Mpu6050Sensor(int sdaPin, int sclPin, String topic_base, PubSubClient* mqttClient, unsigned long interval) 
    : _i2c_bus(new TwoWire(0)) { // Inicializa o ponteiro do barramento I2C
    
    _sdaPin = sdaPin;
    _sclPin = sclPin;
    _client = mqttClient;
    _interval = interval;
    _lastReadTime = 0;
    
    // Tópico único baseado nos pinos SDA/SCL
    _topic = topic_base + "/sda" + String(sdaPin);
}

// --- Implementação do Destrutor ---
Mpu6050Sensor::~Mpu6050Sensor() {
    // Libera a memória alocada para o barramento I2C
    delete _i2c_bus;
}

// --- Implementação do Setup ---
void Mpu6050Sensor::setup() {
    // 1. Inicia o barramento I2C nos pinos especificados
    _i2c_bus->begin(_sdaPin, _sclPin);

    // 2. Inicia o sensor MPU6050 neste barramento
    if (!_mpu.begin(MPU6050_I2CADDR_DEFAULT, _i2c_bus)) {
            Serial.printf("[MPU6050] Erro ao inicializar sensor nos pinos SDA:%d, SCL:%d\n", _sdaPin, _sclPin);
        // (Em um caso real, poderíamos tentar reiniciar ou marcar como falho)
        return;
    }

    // 3. Configura os ranges (opcional, mas recomendado)
    _mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    _mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    _mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    Serial.printf("[MPU6050] Sensor inicializado. SDA:%d, SCL:%d. Publicando em %s\n", _sdaPin, _sclPin, _topic.c_str());
}

// --- Implementação do Loop ---
void Mpu6050Sensor::loop() {
    // Respeita o intervalo de leitura
    if (millis() - _lastReadTime >= _interval) {
        _lastReadTime = millis();

        sensors_event_t a, g, temp;
        // Pega todos os eventos (leituras) de uma vez
        _mpu.getEvent(&a, &g, &temp);

        // --- Cria o Payload JSON ---
        // Aloca 256 bytes para o JSON. Mais que suficiente.
        DynamicJsonDocument doc(256);

        // Adiciona os valores do Acelerômetro
        doc["accel_x"] = a.acceleration.x;
        doc["accel_y"] = a.acceleration.y;
        doc["accel_z"] = a.acceleration.z;

        // Adiciona os valores do Giroscópio
        doc["gyro_x"] = g.gyro.x;
        doc["gyro_y"] = g.gyro.y;
        doc["gyro_z"] = g.gyro.z;
        
        // Adiciona o valor da Temperatura
        doc["temp_c"] = temp.temperature;

        // --- Publica o JSON ---
        char payload[256];
        // Converte o documento JSON em uma string
        serializeJson(doc, payload, sizeof(payload));

        Serial.printf("[MPU6050] SDA %d - Publicando JSON: %s\n", _sdaPin, payload);
        _client->publish(_topic.c_str(), payload);
    }
}

// --- Implementação do getType ---
String Mpu6050Sensor::getType() {
    return "mpu6050";
}