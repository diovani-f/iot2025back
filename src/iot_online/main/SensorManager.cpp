#include "SensorManager.h"
#include "config.h"
#include "Sensor.h"
#include <ArduinoJson.h>

#include "Botao.h"
#include "EncoderSensor.h"
#include "Hcsr04Sensor.h"
#include "IrReceiverSensor.h"
#include "Mpu6050Sensor.h"
#include "KeypadSensor.h"
#include "DhtSensor.h"
#include "Ds18b20Sensor.h"
#include "Apds9960Sensor.h"
#include "JoystickKy023Sensor.h"
#include "MotorVibracao.h"
#include "ReleAtuador.h"
#include "LedAtuador.h"

#include <Adafruit_APDS9960.h>
#include <DHT.h> 

static Sensor* sensores[MAX_SENSORES];
static int numSensores = 0;
static PubSubClient* _mqttClient;
static bool irReceiverActive = false;

void sensorManagerSetup(PubSubClient* client) {
    _mqttClient = client;
    numSensores = 0;
    irReceiverActive = false;
    for(int i=0; i<MAX_SENSORES; i++) {
        sensores[i] = nullptr;
    }
}

void addSensor(JsonObject config) {
    if (numSensores >= MAX_SENSORES) {
        _mqttClient->publish(topic_config_response, "Erro: Maximo de sensores atingido");
        return;
    }

    String tipo = config["tipo"];
    if (tipo.isEmpty()) {
        _mqttClient->publish(topic_config_response, "Erro: 'tipo' nao especificado");
        return;
    }
    
    JsonArray pinArray = config["pinos"];
    
    Serial.printf("[Manager] Tentando adicionar sensor '%s'\n", tipo.c_str());
    
    if (tipo == "botao" || tipo == "encoder" || tipo == "ir_receiver" || 
        tipo == "dht11" || tipo == "dht22" || tipo == "ds18b20" || 
        tipo == "motor_vibracao" || tipo == "rele" || tipo == "led") {
        
        if (pinArray.isNull() || pinArray.size() != 1) {
            String msg = "Erro: " + tipo + " requer array 'pinos' com 1 pino. Ex: [15]";
            _mqttClient->publish(topic_config_response, msg.c_str());
            return;
        }
        
        int pino = pinArray[0];

        // Criação do Objeto
        if (tipo == "botao") {
            sensores[numSensores] = new Botao(pino, "grupoX/sensor/botao", _mqttClient);
        } 
        else if (tipo == "encoder") {
            sensores[numSensores] = new EncoderSensor(pino, "grupoX/sensor/encoder", _mqttClient);
        } 
        else if (tipo == "ir_receiver") {
            if (irReceiverActive) {
                _mqttClient->publish(topic_config_response, "Erro: Apenas um IrReceiver permitido");
                return;
            }
            sensores[numSensores] = new IrReceiverSensor(pino, "grupoX/sensor/ir_receiver", _mqttClient);
            irReceiverActive = true;
        } 
        else if (tipo == "dht11" || tipo == "dht22") {
            uint8_t dht_lib_type = (tipo == "dht11") ? DHT11 : DHT22;
            sensores[numSensores] = new DhtSensor(pino, dht_lib_type, tipo, "grupoX/sensor/dht", _mqttClient);
        } 
        else if (tipo == "ds18b20") {
            sensores[numSensores] = new Ds18b20Sensor(pino, "grupoX/sensor/ds18b20", _mqttClient);
        } 
        else if (tipo == "motor_vibracao") {
            sensores[numSensores] = new MotorVibracao(pino, "grupoX/atuador/vibracao", _mqttClient);
        } 
        else if (tipo == "rele") {
            bool invertido = config.containsKey("invertido") ? config["invertido"].as<bool>() : false;
            sensores[numSensores] = new ReleAtuador(pino, "grupoX/atuador/rele", _mqttClient, invertido);
        } 
        else if (tipo == "led") {
            sensores[numSensores] = new LedAtuador(pino, "grupoX/atuador/led", _mqttClient);
        }

        sensores[numSensores]->setup();
        numSensores++;
        String msg = "OK: " + tipo + " adicionado no pino " + String(pino);
        _mqttClient->publish(topic_config_response, msg.c_str());

    } else if (tipo == "hcsr04" || tipo == "mpu6050" || tipo == "apds9960") {
        
        if (pinArray.isNull() || pinArray.size() != 2) {
            String msg = "Erro: " + tipo + " requer array 'pinos' com 2 pinos. Ex: [21, 22]";
            _mqttClient->publish(topic_config_response, msg.c_str());
            return;
        }
        
        int pino1 = pinArray[0];
        int pino2 = pinArray[1]; // Echo ou SCL
        String msg = "";

        if (tipo == "hcsr04") {
            sensores[numSensores] = new Hcsr04Sensor(pino1, pino2, "grupoX/sensor/hcsr04", _mqttClient);
            msg = "OK: HC-SR04 (Trig:" + String(pino1) + ", Echo:" + String(pino2) + ")";
        } 
        else if (tipo == "mpu6050") {
            sensores[numSensores] = new Mpu6050Sensor(pino1, pino2, "grupoX/sensor/mpu6050", _mqttClient);
            msg = "OK: MPU-6050 (SDA:" + String(pino1) + ", SCL:" + String(pino2) + ")";
        } 
        else if (tipo == "apds9960") {
            sensores[numSensores] = new Apds9960Sensor(pino1, pino2, "grupoX/sensor/apds9960", _mqttClient);
            msg = "OK: APDS-9960 (SDA:" + String(pino1) + ", SCL:" + String(pino2) + ")";
        }

        sensores[numSensores]->setup();
        numSensores++;
        _mqttClient->publish(topic_config_response, msg.c_str());

    } else if (tipo == "joystick_ky023") {
        
        if (pinArray.isNull() || pinArray.size() != 3) {
            _mqttClient->publish(topic_config_response, "Erro: joystick_ky023 requer array 'pinos' com 3 pinos [X, Y, SW]");
            return;
        }
        
        byte* pins = new byte[3];
        pins[0] = pinArray[0];
        pins[1] = pinArray[1];
        pins[2] = pinArray[2];

        sensores[numSensores] = new JoystickKy023Sensor(pins, "grupoX/sensor/joystick", _mqttClient);
        sensores[numSensores]->setup();
        numSensores++;
        String msg = "OK: Joystick adicionado";
        _mqttClient->publish(topic_config_response, msg.c_str());

    } else if (tipo == "keypad4x4") {
        
        if (pinArray.isNull() || pinArray.size() != 8) {
            _mqttClient->publish(topic_config_response, "Erro: keypad4x4 requer array 'pinos' com 8 pinos");
            return;
        }
        
        byte* rowPins = new byte[4];
        byte* colPins = new byte[4];
        for(int i=0; i<4; i++) rowPins[i] = pinArray[i];
        for(int i=0; i<4; i++) colPins[i] = pinArray[i+4];

        sensores[numSensores] = new KeypadSensor(rowPins, colPins, "grupoX/sensor/keypad", _mqttClient);
        sensores[numSensores]->setup();
        numSensores++;
        _mqttClient->publish(topic_config_response, "OK: Keypad 4x4 adicionado");

    } else {
        String msg = "Erro: Tipo de sensor desconhecido '" + tipo + "'";
        _mqttClient->publish(topic_config_response, msg.c_str());
    }
}

void sensorManagerLoop() {
    for (int i = 0; i < numSensores; i++) {
        if (sensores[i] != nullptr) {
            sensores[i]->loop();
        }
    }
}

void sensorManagerHandleMessage(String topic, String payload) {
    for (int i = 0; i < numSensores; i++) {
        if (sensores[i] != nullptr) {
            sensores[i]->handleMqttMessage(topic, payload);
        }
    }
}