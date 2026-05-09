#ifndef COMPONENTES_H
#define COMPONENTES_H

/*
 * ============================================================
 * Componentes.h
 * ------------------------------------------------------------
 * Declarações dos componentes físicos conectados ao ESP32:
 *   - Sensor de temperatura e umidade (DHT22)
 *   - Buzzer para alertas sonoros
 *   - Display LCD I2C
 *
 * As implementações estão em Componentes.cpp.
 * ============================================================
 */

#include <Arduino.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include "DebugManager.h"   // Funções de log: debugInfo() e debugErro()
#include "MqttManager.h"    // Publicação de mensagens via MQTT

// -------------------------------------------------------
// Pinos dos componentes físicos
// Altere conforme a ligação real no seu hardware
// -------------------------------------------------------
#define DHTPIN      10   // Pino de dados do sensor DHT22
#define DHTTYPE     DHT22 // Modelo do sensor de temperatura/umidade
#define BUZZER_PIN  8    // Pino do buzzer (saída digital)

// -------------------------------------------------------
// Faixas seguras de temperatura e umidade
// Baseadas na ABNT NBR 9077 e ISO 11799 (salas de arquivo)
// Valores fora dessa faixa disparam o alarme
// -------------------------------------------------------
#define TEMP_MIN    21.0f  // Temperatura mínima segura (°C)
#define TEMP_MAX    25.0f  // Temperatura máxima segura (°C)
#define UMID_MIN    45.0f  // Umidade mínima segura (%)
#define UMID_MAX    60.0f  // Umidade máxima segura (%)

// -------------------------------------------------------
// Índices dos tópicos de publicação MQTT
// Correspondem às posições do array TOPICOS_PUBLICAR[]
// definido em secrets.cpp
//   [0] = comandoLampada
//   [1] = comandoUmidade
//   [2] = comandoTemperatura
//   [3] = statusAlarme
// -------------------------------------------------------
#define INDICE_TOPICO_TEMPERATURA 2  // Publica leituras de temperatura
#define INDICE_TOPICO_UMIDADE     1  // Publica leituras de umidade
#define INDICE_TOPICO_ALARME      3  // Publica status do alarme (0 = normal, 1 = alerta)

// -------------------------------------------------------
// Limite de falhas consecutivas do sensor DHT
// Após atingir esse valor, o erro é logado apenas uma vez
// para evitar poluição excessiva no serial/debug
// -------------------------------------------------------
#define MAX_FALHAS_DHT 5

// -------------------------------------------------------
// Instâncias externas dos componentes físicos
// Definidas em Componentes.cpp — acessíveis em qualquer
// arquivo que inclua este header
// -------------------------------------------------------
extern int               buzzer;  // Representa o pino do buzzer
extern DHT               dht;     // Objeto do sensor DHT22
extern LiquidCrystal_I2C lcd;     // Objeto do display LCD via I2C

// -------------------------------------------------------
// Inicializa todos os componentes (DHT, LCD, Buzzer)
// Deve ser chamada UMA VEZ dentro do setup()
// -------------------------------------------------------
void setupComponentes();

// -------------------------------------------------------
// Realiza a leitura do DHT22, publica temperatura e umidade
// via MQTT e atualiza o display LCD com os valores atuais.
// Operação não-bloqueante — deve ser chamada no loop()
// -------------------------------------------------------
void verificarTemperaturaEUmidade();

#endif