#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

/*
 * ============================================================
 * MqttManager.h
 * ------------------------------------------------------------
 * Camada de abstração para comunicação MQTT do ESP32.
 * Gerencia conexão, reconexão, publicação e recebimento
 * de mensagens com o broker definido em secrets.cpp.
 *
 * Suporta TLS (HiveMQ Cloud) e conexão sem criptografia.
 * A lógica de implementação está em MqttManager.cpp.
 * ============================================================
 */

#include <Arduino.h>

// -------------------------------------------------------
// Tipo do callback de recebimento de mensagens MQTT.
// A aplicação registra uma função desse tipo via
// registrarCallbackMensagem() para processar as mensagens
// que chegam em qualquer tópico assinado.
//
// Parâmetros:
//   topico   — nome do tópico onde a mensagem chegou
//   mensagem — conteúdo da mensagem (payload)
// -------------------------------------------------------
typedef void (*CallbackMensagemMQTT)(const char* topico, const String& mensagem);

// -------------------------------------------------------
// Configura o cliente MQTT com broker, porta, TLS e
// certificado CA definidos em secrets.cpp.
// Deve ser chamada UMA VEZ no setup(), após conectarWiFi()
// -------------------------------------------------------
void configurarMQTT();

// -------------------------------------------------------
// Realiza a conexão ao broker MQTT com as credenciais
// definidas em secrets.cpp (usuário, senha, client ID).
// Assina automaticamente todos os tópicos de TOPICOS_RECEBER[]
// após conectar com sucesso
// -------------------------------------------------------
void conectarMQTT();

// -------------------------------------------------------
// Verifica se o MQTT ainda está conectado e reconecta
// automaticamente se necessário.
// DEVE ser chamada a cada iteração do loop()
// -------------------------------------------------------
void garantirMQTTConectado();

// -------------------------------------------------------
// Processa as mensagens MQTT recebidas pelo broker e
// dispara o callback registrado via registrarCallbackMensagem().
// DEVE ser chamada a cada iteração do loop()
// -------------------------------------------------------
void loopMQTT();

// -------------------------------------------------------
// Publica uma mensagem em um tópico MQTT pelo nome completo.
//
// Parâmetros:
//   topico    — string com o caminho completo do tópico
//   mensagem  — payload a ser enviado (texto)
// -------------------------------------------------------
void publicarMensagem(const char* topico, const char* mensagem);

// -------------------------------------------------------
// Publica uma mensagem usando o índice do array
// TOPICOS_PUBLICAR[] definido em secrets.cpp.
// Evita hardcode de strings de tópico espalhadas pelo código.
//
// Parâmetros:
//   indiceTopico — posição no array TOPICOS_PUBLICAR[]
//   mensagem     — payload a ser enviado (texto)
// -------------------------------------------------------
void publicarMensagemNoTopico(int indiceTopico, const char* mensagem);

// -------------------------------------------------------
// Retorna o tópico de publicação pelo índice.
// Consulta o array TOPICOS_PUBLICAR[] em secrets.cpp
// -------------------------------------------------------
const char* obterTopicoPublicacao(int indiceTopico);

// -------------------------------------------------------
// Retorna o tópico de recebimento (assinatura) pelo índice.
// Consulta o array TOPICOS_RECEBER[] em secrets.cpp
// -------------------------------------------------------
const char* obterTopicoRecebimento(int indiceTopico);

// -------------------------------------------------------
// Retorna o total de tópicos que o ESP assina (TOTAL_TOPICOS_RECEBER).
// Usado internamente para iterar sobre as assinaturas
// -------------------------------------------------------
int obterTotalTopicosRecebimento();

// -------------------------------------------------------
// Registra o callback da aplicação que será chamado
// automaticamente ao receber qualquer mensagem MQTT.
// Deve ser chamado no setup() antes de conectarMQTT()
//
// Parâmetro:
//   callback — ponteiro para função do tipo
//              void meuCallback(const char* topico, const String& mensagem)
// -------------------------------------------------------
void registrarCallbackMensagem(CallbackMensagemMQTT callback);

// -------------------------------------------------------
// Retorna true se o cliente MQTT estiver conectado
// ao broker no momento da chamada
// -------------------------------------------------------
bool mqttEstaConectado();

#endif