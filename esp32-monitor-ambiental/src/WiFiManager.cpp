/*
 * ============================================================
 * WiFiManager.cpp
 * ------------------------------------------------------------
 * Gerencia a conexão WiFi do ESP32 usando as credenciais
 * definidas em secrets.cpp.
 *
 * Fluxo esperado:
 *   setup() → conectarWiFi()
 *   loop()  → garantirWiFiConectado()
 * ============================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "WiFiManager.h"
#include "secrets.h"
#include "DebugManager.h"

// -------------------------------------------------------
// Verifica o estado atual da conexão WiFi.
// Consulta diretamente o status da biblioteca WiFi.
//
// Retorna:
//   true  → conectado e com IP válido
//   false → desconectado, sem IP ou em processo de conexão
// -------------------------------------------------------
bool wifiEstaConectado()
{
    return WiFi.status() == WL_CONNECTED;
}

// -------------------------------------------------------
// Conecta ao WiFi usando WIFI_SSID e WIFI_SENHA (secrets.cpp).
//
// Fluxo:
//   1. Define modo station (cliente de roteador)
//   2. Inicia a conexão com as credenciais
//   3. Aguarda em loop não-destrutivo (até 15 segundos)
//   4. Loga o IP obtido em caso de sucesso
//   5. Loga erro claro em caso de falha
//
// Deve ser chamada UMA VEZ no setup().
// -------------------------------------------------------
void conectarWiFi()
{
    // Passo 1 — Anuncia a tentativa de conexão
    debugInfo("Iniciando conexao Wi-Fi...");
    debugInfo("SSID: " + String(WIFI_SSID));

    // Passo 2 — Configura modo station e inicia a conexão
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_SENHA);

    int tentativas        = 0;
    const int maxTentativas = 30; // 30 × 500ms = 15 segundos de espera máxima

    // Passo 3 — Aguarda a conexão com feedback visual no serial
    debugInfoSemLinha("[INFO] Aguardando conexao");
    while (WiFi.status() != WL_CONNECTED && tentativas < maxTentativas)
    {
        delay(500);
        debugInfoSemLinha(".");
        tentativas++;
    }
    debugInfoSemLinha("\n\r");

    // Passo 4 — Avalia e loga o resultado
    if (WiFi.status() == WL_CONNECTED)
    {
        debugInfo("Wi-Fi conectado com sucesso!");
        debugInfo("IP atribuido: " + WiFi.localIP().toString());
        debugInfo("Sinal (RSSI): " + String(WiFi.RSSI()) + " dBm");
    }
    else
    {
        // Passo 5 — Falha: loga com código de status para diagnóstico
        debugErro("Falha ao conectar ao Wi-Fi apos " + String(tentativas) + " tentativas.");
        debugErro("Status WiFi: " + String(WiFi.status()) +
            " (1=sem SSID, 3=conectado, 4=falha, 6=desconectado)");
        debugErro("Verifique SSID e senha em secrets.cpp.");
    }
}

// -------------------------------------------------------
// Garante que o WiFi permanece conectado durante o loop.
// Se detectar queda de conexão, tenta reconectar.
//
// Deve ser chamada A CADA iteração do loop().
// -------------------------------------------------------
void garantirWiFiConectado()
{
    // Verifica silenciosamente — só age se houver problema
    if (!wifiEstaConectado())
    {
        debugErro("Wi-Fi desconectado! Tentando reconectar...");
        conectarWiFi();

        // Confirma o resultado da reconexão
        if (wifiEstaConectado())
            debugInfo("Wi-Fi reconectado com sucesso.");
        else
            debugErro("Reconexao falhou. Proxima tentativa no proximo loop.");
    }
}