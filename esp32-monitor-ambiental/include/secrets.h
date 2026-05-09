#ifndef SECRETS_H
#define SECRETS_H

/*
 * ============================================================
 * secrets.h
 * ------------------------------------------------------------
 * Declarações (extern) de todas as credenciais, configurações
 * de rede, tópicos MQTT e parâmetros de comportamento do sistema.
 *
 * Os VALORES reais estão definidos em secrets.cpp.
 * Separa credenciais sensíveis do restante do código,
 * facilitando o versionamento seguro (secrets.cpp no .gitignore)
 * ============================================================
 */

// -------------------------------------------------------
// Credenciais da rede WiFi
// Definidas em secrets.cpp
// -------------------------------------------------------
extern const char* WIFI_SSID;   // Nome da rede (SSID)
extern const char* WIFI_SENHA;  // Senha da rede

// -------------------------------------------------------
// Configurações do broker MQTT
// Compatível com HiveMQ Cloud (TLS porta 8883)
// e brokers locais sem TLS (porta 1883)
// -------------------------------------------------------
extern const char* MQTT_BROKER;        // Endereço do broker (domínio ou IP)
extern const int   MQTT_PORTA;         // Porta de conexão (8883 TLS / 1883 sem TLS)
extern const char* MQTT_CLIENT_ID;     // ID único do cliente MQTT no broker
extern const char* MQTT_USUARIO;       // Usuário para autenticação no broker
extern const char* MQTT_SENHA;         // Senha para autenticação no broker
extern const bool  MQTT_USAR_TLS;      // true = conexão criptografada com certificado CA
extern const char  MQTT_CERTIFICADO_CA[]; // Certificado CA em formato PEM para validar TLS

// -------------------------------------------------------
// Tópicos MQTT de publicação
// Array com os tópicos que o ESP publica (envia dados).
// Os índices são mapeados pelos defines em Componentes.h:
//   [0] = comandoLampada
//   [1] = comandoUmidade
//   [2] = comandoTemperatura
//   [3] = statusAlarme
// -------------------------------------------------------
extern const char* TOPICOS_PUBLICAR[];      // Array de tópicos de publicação
extern const int   TOTAL_TOPICOS_PUBLICAR;  // Tamanho do array acima

// -------------------------------------------------------
// Tópicos MQTT de recebimento (assinatura)
// Array com os tópicos que o ESP escuta (recebe dados).
// Pode conter wildcards: "#" escuta todos os subtópicos,
// "+" substitui um único nível de hierarquia
// -------------------------------------------------------
extern const char* TOPICOS_RECEBER[];      // Array de tópicos assinados
extern const int   TOTAL_TOPICOS_RECEBER;  // Tamanho do array acima

// -------------------------------------------------------
// Flags de comportamento do sistema
// -------------------------------------------------------
extern const bool USAR_AWS_IOT; // true = usa AWS IoT Core (desativado, usando HiveMQ)

// -------------------------------------------------------
// Configurações do sistema de debug serial
// -------------------------------------------------------
extern const int DEBUG_NIVEL_INICIAL;            // Nível de log ao iniciar (0, 1 ou 2)
extern const int PINO_HABILITA_DEBUG_COMPLETO;   // Pino que, em LOW, força DEBUG_TUDO

#endif