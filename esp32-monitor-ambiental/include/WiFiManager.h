#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

/*
 * ============================================================
 * WiFiManager.h
 * ------------------------------------------------------------
 * Gerenciamento da conexão WiFi do ESP32.
 * Abstrai a lógica de conexão, reconexão e verificação
 * de status, usando as credenciais definidas em secrets.cpp.
 *
 * A implementação está em WiFiManager.cpp.
 *
 * Fluxo esperado:
 *   setup()  → conectarWiFi()
 *   loop()   → garantirWiFiConectado()
 * ============================================================
 */

// -------------------------------------------------------
// Verifica o estado atual da conexão WiFi.
// Retorna true  → conectado e com IP válido
// Retorna false → desconectado ou sem IP
// -------------------------------------------------------
bool wifiEstaConectado();

// -------------------------------------------------------
// Realiza a conexão ao WiFi usando WIFI_SSID e WIFI_SENHA
// definidos em secrets.cpp.
// Aguarda a conexão em loop bloqueante e loga cada tentativa
// com debugInfoSemLinha(".") até obter sucesso.
// Deve ser chamada UMA VEZ no setup()
// -------------------------------------------------------
void conectarWiFi();

// -------------------------------------------------------
// Verifica se o WiFi ainda está conectado e, caso não
// esteja, chama conectarWiFi() para reconectar.
// DEVE ser chamada a cada iteração do loop() para garantir
// resiliência em caso de queda de sinal ou timeout do roteador
// -------------------------------------------------------
void garantirWiFiConectado();

#endif