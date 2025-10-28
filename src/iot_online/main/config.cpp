#include "config.h" // Inclui as declarações extern para checagem

// --- Definições ---
// Aqui é onde as variáveis realmente "vivem" na memória
// e recebem seus valores.

const char* mqtt_server = "wa2fc908.ala.us-east-1.emqxsl.com";
const int mqtt_port = 8883; 
const char* mqtt_user = "diovani";
const char* mqtt_password = "facco123";

const char* client_id = "esp32-dlsc808-grupoX-01";
const char* topic_status = "grupoX/status";
const char* topic_config = "grupoX/config";
const char* topic_config_response = "grupoX/config/response";

// Note que MAX_SENSORES não precisa vir aqui, pois é um #define