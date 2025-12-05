#pragma once

extern const char* mqtt_server;
extern const int mqtt_port; 
extern const char* mqtt_user;
extern const char* mqtt_password;

extern const char* client_id;
extern const char* topic_status;
extern const char* topic_config;
extern const char* topic_config_response;

#define MAX_SENSORES 30