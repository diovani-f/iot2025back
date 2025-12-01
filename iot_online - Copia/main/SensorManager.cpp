#include "SensorManager.h"
#include "config.h"
#include "Sensor.h"
#include <ArduinoJson.h>

// --- Lista de sensores ---
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

// --- Estrutura para o Registro de Duplicidade ---
struct SensorRegistryItem {
    String type;
    int pins[8]; // Suporta até 8 pinos (Keypad)
    int pinCount;
    bool active;
};

// --- Variáveis Internas ---
static Sensor* sensores[MAX_SENSORES];
static SensorRegistryItem registry[MAX_SENSORES]; // Lista paralela para verificação
static int numSensores = 0;
static PubSubClient* _mqttClient;
static bool irReceiverActive = false;
static String _deviceId;

// --- Setup do Manager ---
void sensorManagerSetup(PubSubClient* client, String deviceId) {
    _mqttClient = client;
    _deviceId = deviceId;
    numSensores = 0;
    irReceiverActive = false;
    
    // Limpa os arrays
    for(int i=0; i<MAX_SENSORES; i++) {
        sensores[i] = nullptr;
        registry[i].active = false;
        registry[i].pinCount = 0;
    }
}

// --- Função Auxiliar: Verifica se já existe ---
bool sensorJaExiste(String tipo, JsonArray pinArray) {
    int qtdPinosChegando = pinArray.size();

    // Varre o registro atual
    for(int i=0; i < numSensores; i++) {
        // 1. Verifica se o slot está ativo e se o tipo é o mesmo
        if (registry[i].active && registry[i].type == tipo) {
            
            // 2. Verifica se a quantidade de pinos é a mesma
            if (registry[i].pinCount == qtdPinosChegando) {
                
                // 3. Verifica se TODOS os pinos são iguais
                bool pinosIguais = true;
                for(int j=0; j < qtdPinosChegando; j++) {
                    if (registry[i].pins[j] != pinArray[j].as<int>()) {
                        pinosIguais = false;
                        break;
                    }
                }

                // Se passou por tudo, é duplicado!
                if (pinosIguais) {
                    return true;
                }
            }
        }
    }
    return false;
}

// --- Função Auxiliar: Salva no Registro ---
void registrarSensor(String tipo, JsonArray pinArray) {
    if (numSensores < MAX_SENSORES) {
        registry[numSensores].type = tipo;
        registry[numSensores].pinCount = pinArray.size();
        registry[numSensores].active = true;
        
        for(int i=0; i < pinArray.size(); i++) {
            if (i < 8) registry[numSensores].pins[i] = pinArray[i];
        }
    }
}

// --- Função Factory Principal ---
void addSensor(JsonObject config) {
    String tipo = config["tipo"];
    if (tipo.isEmpty()) {
        _mqttClient->publish(topic_config_response, "Erro: 'tipo' nao especificado");
        return;
    }
    
    JsonArray pinArray = config["pinos"];
    
    // --- PASSO 1: Verifica Duplicidade ---
    if (sensorJaExiste(tipo, pinArray)) {
        Serial.printf("[Manager] Ignorando duplicado: %s\n", tipo.c_str());
        // Envia OK para o backend não achar que deu erro, mas avisa que já existe
        String msg = "OK: " + tipo + " ja existe (ignorado)";
        _mqttClient->publish(topic_config_response, msg.c_str());
        return; 
    }

    // --- PASSO 2: Verifica Limite ---
    if (numSensores >= MAX_SENSORES) {
        _mqttClient->publish(topic_config_response, "Erro: Maximo atingido");
        return;
    }

    Serial.printf("[Manager] Adicionando novo: %s\n", tipo.c_str());
    String topicPrefix = "grupoX/" + _deviceId + "/atuador"; 

    // =================================================================================
    // 1 PINO
    // =================================================================================
    if (tipo == "botao" || tipo == "encoder" || tipo == "ir_receiver" || 
        tipo == "dht11" || tipo == "dht22" || tipo == "ds18b20" || 
        tipo == "motor_vibracao" || tipo == "rele" || tipo == "led") {
        
        if (pinArray.isNull() || pinArray.size() != 1) {
            _mqttClient->publish(topic_config_response, "Erro: Requer 1 pino");
            return;
        }
        int pino = pinArray[0];

        if (tipo == "botao") sensores[numSensores] = new Botao(pino, "grupoX/sensor/botao", _mqttClient);
        else if (tipo == "encoder") sensores[numSensores] = new EncoderSensor(pino, "grupoX/sensor/encoder", _mqttClient);
        else if (tipo == "ir_receiver") {
            if (irReceiverActive) { _mqttClient->publish(topic_config_response, "Erro: IR ja ativo"); return; }
            sensores[numSensores] = new IrReceiverSensor(pino, "grupoX/sensor/ir_receiver", _mqttClient);
            irReceiverActive = true;
        } 
        else if (tipo == "dht11" || tipo == "dht22") {
            uint8_t dht_lib_type = (tipo == "dht11") ? DHT11 : DHT22;
            sensores[numSensores] = new DhtSensor(pino, dht_lib_type, tipo, "grupoX/sensor/dht", _mqttClient);
        } 
        else if (tipo == "ds18b20") sensores[numSensores] = new Ds18b20Sensor(pino, "grupoX/sensor/ds18b20", _mqttClient);
        else if (tipo == "motor_vibracao") sensores[numSensores] = new MotorVibracao(pino, topicPrefix + "/vibracao", _mqttClient);
        else if (tipo == "rele") {
            bool invertido = config.containsKey("invertido") ? config["invertido"].as<bool>() : false;
            sensores[numSensores] = new ReleAtuador(pino, topicPrefix + "/rele", _mqttClient, invertido);
        } 
        else if (tipo == "led") sensores[numSensores] = new LedAtuador(pino, topicPrefix + "/led", _mqttClient);

        // --- REGISTRA E INCREMENTA ---
        sensores[numSensores]->setup();
        registrarSensor(tipo, pinArray); // <--- Salva no registro
        numSensores++;
        
        String msg = "OK: " + tipo + " adicionado no pino " + String(pino);
        _mqttClient->publish(topic_config_response, msg.c_str());

    // =================================================================================
    // 2 PINOS
    // =================================================================================
    } else if (tipo == "hcsr04" || tipo == "mpu6050" || tipo == "apds9960") {
        if (pinArray.isNull() || pinArray.size() != 2) {
            _mqttClient->publish(topic_config_response, "Erro: Requer 2 pinos");
            return;
        }
        int p1 = pinArray[0]; int p2 = pinArray[1];

        if (tipo == "hcsr04") sensores[numSensores] = new Hcsr04Sensor(p1, p2, "grupoX/sensor/hcsr04", _mqttClient);
        else if (tipo == "mpu6050") sensores[numSensores] = new Mpu6050Sensor(p1, p2, "grupoX/sensor/mpu6050", _mqttClient);
        else if (tipo == "apds9960") sensores[numSensores] = new Apds9960Sensor(p1, p2, "grupoX/sensor/apds9960", _mqttClient);

        sensores[numSensores]->setup();
        registrarSensor(tipo, pinArray);
        numSensores++;
        _mqttClient->publish(topic_config_response, "OK: Sensor duplo adicionado");

    // =================================================================================
    // 3 PINOS (Joystick)
    // =================================================================================
    } else if (tipo == "joystick_ky023") {
        if (pinArray.isNull() || pinArray.size() != 3) {
            _mqttClient->publish(topic_config_response, "Erro: Requer 3 pinos");
            return;
        }
        byte* pins = new byte[3]; pins[0]=pinArray[0]; pins[1]=pinArray[1]; pins[2]=pinArray[2];
        sensores[numSensores] = new JoystickKy023Sensor(pins, "grupoX/sensor/joystick", _mqttClient);
        
        sensores[numSensores]->setup();
        registrarSensor(tipo, pinArray);
        numSensores++;
        _mqttClient->publish(topic_config_response, "OK: Joystick adicionado");

    // =================================================================================
    // 8 PINOS (Teclado)
    // =================================================================================
    } else if (tipo == "keypad4x4") {
        if (pinArray.isNull() || pinArray.size() != 8) {
            _mqttClient->publish(topic_config_response, "Erro: Requer 8 pinos");
            return;
        }
        byte* r = new byte[4]; byte* c = new byte[4];
        for(int i=0;i<4;i++) r[i]=pinArray[i];
        for(int i=0;i<4;i++) c[i]=pinArray[i+4];
        sensores[numSensores] = new KeypadSensor(r, c, "grupoX/sensor/keypad", _mqttClient);
        
        sensores[numSensores]->setup();
        registrarSensor(tipo, pinArray);
        numSensores++;
        _mqttClient->publish(topic_config_response, "OK: Keypad adicionado");

    } else {
        _mqttClient->publish(topic_config_response, "Erro: Tipo desconhecido");
    }
}

// --- Loops ---
void sensorManagerLoop() {
    for (int i = 0; i < numSensores; i++) {
        if (sensores[i] != nullptr) sensores[i]->loop();
    }
}

void sensorManagerHandleMessage(String topic, String payload) {
    for (int i = 0; i < numSensores; i++) {
        if (sensores[i] != nullptr) sensores[i]->handleMqttMessage(topic, payload);
    }
}