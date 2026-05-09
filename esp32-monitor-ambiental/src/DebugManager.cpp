/*
 * ============================================================
 * DebugManager.cpp
 * ------------------------------------------------------------
 * Implementação do sistema de log serial do projeto.
 * Controla a verbosidade da saída com base no nível configurado
 * em secrets.cpp (DEBUG_NIVEL_INICIAL) e no estado do pino
 * físico PINO_HABILITA_DEBUG_COMPLETO.
 *
 * Níveis disponíveis:
 *   DEBUG_NENHUM (0) → sem saída serial
 *   DEBUG_ERRO   (1) → somente erros
 *   DEBUG_TUDO   (2) → erros + informações
 * ============================================================
 */

#include <Arduino.h>
#include "DebugManager.h"
#include "secrets.h"

// Nível de debug ativo em tempo de execução.
// Inicializado com o valor de secrets.cpp e pode ser
// sobrescrito pelo pino físico em configurarDebug().
int nivelDebugAtual = DEBUG_NIVEL_INICIAL;

// -------------------------------------------------------
// Imprime uma mensagem de ERRO no serial com prefixo [ERRO].
// Só exibe se o nível atual for DEBUG_ERRO (1) ou maior.
// -------------------------------------------------------
void debugErro(const String& mensagem)
{
    if (nivelDebugAtual >= DEBUG_ERRO)
    {
        Serial.print("[ERRO] ");
        Serial.println(mensagem);
    }
}

// -------------------------------------------------------
// Imprime uma mensagem de ERRO sem quebra de linha no final.
// Útil para compor mensagens progressivas na mesma linha.
// Só exibe se o nível atual for DEBUG_ERRO (1) ou maior.
// -------------------------------------------------------
void debugErroSemLinha(const String& mensagem)
{
    if (nivelDebugAtual >= DEBUG_ERRO)
    {
        Serial.print(mensagem);
    }
}

// -------------------------------------------------------
// Imprime uma mensagem INFORMATIVA no serial com prefixo [INFO].
// Só exibe se o nível atual for DEBUG_TUDO (2).
// -------------------------------------------------------
void debugInfo(const String& mensagem)
{
    if (nivelDebugAtual >= DEBUG_TUDO)
    {
        Serial.print("[INFO] ");
        Serial.println(mensagem);
    }
}

// -------------------------------------------------------
// Imprime uma mensagem INFORMATIVA sem quebra de linha.
// Útil para compor linhas como: "Conectando....OK!"
// Só exibe se o nível atual for DEBUG_TUDO (2).
// -------------------------------------------------------
void debugInfoSemLinha(const String& mensagem)
{
    if (nivelDebugAtual >= DEBUG_TUDO)
    {
        Serial.print(mensagem);
    }
}

// -------------------------------------------------------
// Retorna o nível de debug ativo no momento.
// Pode ser consultado por outros módulos se necessário.
// -------------------------------------------------------
int obterNivelDebugAtual()
{
    return nivelDebugAtual;
}

// -------------------------------------------------------
// Inicializa o sistema de debug:
//   1. Inicia a porta Serial a 9600 baud
//   2. Aguarda 500ms para estabilizar
//   3. Configura o pino físico como entrada com pull-up
//   4. Se o pino estiver em LOW (jumper/botão pressionado),
//      força DEBUG_TUDO independente de secrets.cpp
//   5. Loga o estado inicial do sistema
//
// DEVE ser a primeira função chamada no setup() para que
// todos os logs subsequentes sejam capturados corretamente.
// -------------------------------------------------------
void configurarDebug()
{
    // Passo 1 — Inicializa a comunicação serial
    Serial.begin(9600);
    delay(500); // Aguarda estabilização da porta serial

    // Passo 2 — Configura o pino que permite forçar debug completo
    // em campo sem recompilar o firmware
    pinMode(PINO_HABILITA_DEBUG_COMPLETO, INPUT_PULLUP);

    // Passo 3 — Define o nível ativo:
    //   pino em LOW  → forçado para DEBUG_TUDO (jumper físico inserido)
    //   pino em HIGH → usa o valor padrão de secrets.cpp
    nivelDebugAtual = (digitalRead(PINO_HABILITA_DEBUG_COMPLETO) == LOW)
        ? DEBUG_TUDO
        : DEBUG_NIVEL_INICIAL;

    // Passo 4 — Loga o resultado da inicialização
    debugInfo("=== ESP32 INICIADO ===");
    debugInfo("Nivel de debug ativo: " +
        String(nivelDebugAtual == DEBUG_NENHUM ? "NENHUM" :
               nivelDebugAtual == DEBUG_ERRO   ? "APENAS ERROS" : "COMPLETO")
    );
    debugInfo("Pino de debug fisico (GPIO " +
        String(PINO_HABILITA_DEBUG_COMPLETO) + "): " +
        String(digitalRead(PINO_HABILITA_DEBUG_COMPLETO) == LOW
            ? "LOW → debug forcado"
            : "HIGH → usando padrao")
    );
}