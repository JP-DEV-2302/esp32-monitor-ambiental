// Copie este arquivo para secrets.cpp e preencha com seus dados reais
#include "secrets.h"
#include <Arduino.h>

const char* WIFI_SSID  = "SUA_REDE_WIFI";
const char* WIFI_SENHA = "SUA_SENHA_WIFI";

const char* MQTT_BROKER    = "seu-broker.hivemq.cloud";
const int   MQTT_PORTA     = 8883;
const char* MQTT_CLIENT_ID = "SeuClienteId";
const char* MQTT_USUARIO   = "SeuUsuario";
const char* MQTT_SENHA     = "SuaSenha";
const bool  MQTT_USAR_TLS  = true;

const char MQTT_CERTIFICADO_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
Cole aqui seu certificado CA
-----END CERTIFICATE-----
)EOF";

const char* TOPICOS_PUBLICAR[] = {
    "prefixo/dispositivo/comandoLampada",
    "prefixo/dispositivo/comandoUmidade",
    "prefixo/dispositivo/comandoTemperatura",
    "prefixo/dispositivo/statusAlarme"
};
const int TOTAL_TOPICOS_PUBLICAR = 4;

const char* TOPICOS_RECEBER[] = {
    "prefixo/dispositivo/#"
};
const int TOTAL_TOPICOS_RECEBER = 1;

const bool USAR_AWS_IOT               = false;
const int  DEBUG_NIVEL_INICIAL        = 2;
const int  PINO_HABILITA_DEBUG_COMPLETO = 4;