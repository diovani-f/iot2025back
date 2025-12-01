#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "config.h"         
#include "SensorManager.h"    

WiFiClientSecure espClient;
PubSubClient client(espClient);

// --- Variáveis Dinâmicas de Identidade ---
String myDeviceId;       // Ex: "esp32-A1B2C3"
String myConfigTopic;    // Ex: "grupoX/esp32-A1B2C3/config"
String myStatusTopic;    // Ex: "grupoX/esp32-A1B2C3/status"

// Função de Callback (Recebe mensagens)
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Mensagem recebida no tópico: ");
    Serial.println(topic);
    
    payload[length] = '\0';
    String msg = (char*)payload;

    // Verifica se é tópico de config (Global OU Específico deste ESP)
    if (String(topic) == topic_config || String(topic) == myConfigTopic) {
        
        DynamicJsonDocument doc(1024); // Aumentei o buffer para garantir
        DeserializationError error = deserializeJson(doc, payload, length);

        if (error) {
            Serial.println("Erro: JSON invalido");
            return;
        }

        JsonObject config = doc.as<JsonObject>();
        
        if (!config.containsKey("comando")) return;
        
        String comando = config["comando"]; 

        if (comando == "RESTART") {
            Serial.println("Recebido comando de RESTART. Reiniciando em 1s...");
            client.publish(topic_config_response, "OK: Reiniciando dispositivo...");
            delay(1000); 
            ESP.restart();
        } else if (comando == "ADD") {
            addSensor(config);
        }
        
    } else {
        // Se não é config, é mensagem para Atuador (ex: Ligar LED)
        // Repassa para o gerenciador, que sabe qual sensor deve receber
        sensorManagerHandleMessage(String(topic), msg);
    }
}

// Função de Reconexão MQTT
void reconnect_mqtt() {
    while (!client.connected()) {
        Serial.print("Conectando ao MQTT como: ");
        Serial.print(myDeviceId);
        Serial.print(" ... ");

        // Usa o ID Dinâmico para conectar
        if (client.connect(myDeviceId.c_str(), mqtt_user, mqtt_password)) {
            Serial.println("SUCESSO!");
            
            // 1. Publica status online no tópico específico (Para o Auto-Discovery do Backend)
            client.publish(myStatusTopic.c_str(), "online", true);
            
            // 2. Inscreve no Tópico GLOBAL (para configurar todos de uma vez)
            client.subscribe(topic_config);
            Serial.println("Inscrito Global: " + String(topic_config));
            
            // 3. Inscreve no Tópico ESPECÍFICO (para configurar só este ESP)
            client.subscribe(myConfigTopic.c_str());
            Serial.println("Inscrito Específico: " + myConfigTopic);

            // 4. Inscreve nos atuadores ESPECÍFICOS deste dispositivo
            // Formato: grupoX/[ID]/atuador/#
            String topicAtuadores = "grupoX/" + myDeviceId + "/atuador/#";
            client.subscribe(topicAtuadores.c_str());
            Serial.println("Inscrito Atuadores: " + topicAtuadores);

        } else {
            Serial.print("falhou, rc=");
            Serial.print(client.state());
            Serial.println(" tentando em 5s");
            delay(5000);
        }
    }
}

// --- SETUP PRINCIPAL ---
void setup() {
    Serial.begin(115200);
    Serial.println("\n\n--- INICIANDO SISTEMA ---");

    // 1. CONEXÃO WI-FI (IMPORTANTE: Deve vir antes de gerar o ID)
    // Precisamos ligar o rádio Wi-Fi para que o MAC Address seja lido corretamente
    WiFiManager wm;
    // wm.resetSettings(); // Descomente se precisar limpar o Wi-Fi salvo
    
    if (!wm.autoConnect("Plataforma-IoT-Config", "senha1234")) {
        Serial.println("Falha ao conectar ao Wi-Fi. Reiniciando...");
        delay(3000);
        ESP.restart();
    }
    Serial.println("\n--- WI-FI CONECTADO! ---");

    // 2. GERAÇÃO DE ID ÚNICO
    // Agora que o Wi-Fi está ligado, o MAC é válido e único.
    String mac = WiFi.macAddress();
    mac.replace(":", ""); 
    String shortMac = mac.substring(6); // Pega os últimos 6 caracteres (ex: A1B2C3)
    
    myDeviceId = "esp32-" + shortMac;
    
    // Define os tópicos baseados no ID
    String prefixo = "grupoX"; 
    myConfigTopic = prefixo + "/" + myDeviceId + "/config";
    myStatusTopic = prefixo + "/" + myDeviceId + "/status";

    Serial.println("---------------------------");
    Serial.print("ID DO DISPOSITIVO: ");
    Serial.println(myDeviceId);
    Serial.print("TOPICO CONFIG:     ");
    Serial.println(myConfigTopic);
    Serial.println("---------------------------");

    // 3. CONFIGURAÇÃO MQTT
    espClient.setInsecure(); // Aceita certificados autoassinados/inválidos
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback); 

    // 4. INICIALIZA O GERENCIADOR
    // Passamos o cliente MQTT e o ID único deste dispositivo
    sensorManagerSetup(&client, myDeviceId);
}

void loop() {
    if (!client.connected()) {
        reconnect_mqtt();
    }
    client.loop(); 
    
    // Roda o loop de todos os sensores ativos
    sensorManagerLoop();
    
    delay(10); // Pequeno delay para estabilidade
}