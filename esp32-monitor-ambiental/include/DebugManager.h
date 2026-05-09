#ifndef DEBUG_MANAGER_H
#define DEBUG_MANAGER_H

/*
 * ============================================================
 * DebugManager.h
 * ------------------------------------------------------------
 * Sistema centralizado de log serial para o projeto.
 * Permite controlar a verbosidade dos logs em tempo de
 * compilação (via secrets.h) e em tempo de execução
 * (via pino físico definido em secrets.h).
 *
 * Uso:
 *   debugInfo("Mensagem informativa");
 *   debugErro("Algo deu errado: " + String(codigo));
 * ============================================================
 */

#include <Arduino.h>

// -------------------------------------------------------
// Níveis de verbosidade do log serial
// Defina DEBUG_NIVEL_INICIAL em secrets.cpp para controlar
// o comportamento padrão ao iniciar o sistema
// -------------------------------------------------------
#define DEBUG_NENHUM 0  // Sem saída serial (produção silenciosa)
#define DEBUG_ERRO   1  // Exibe apenas erros críticos
#define DEBUG_TUDO   2  // Exibe erros + informações de fluxo

// -------------------------------------------------------
// Inicializa a porta Serial e aplica o nível de debug
// definido em DEBUG_NIVEL_INICIAL (secrets.cpp).
// Deve ser chamada como PRIMEIRA instrução do setup()
// para garantir que os logs seguintes sejam capturados
// -------------------------------------------------------
void configurarDebug();

// -------------------------------------------------------
// Exibe uma mensagem de erro no serial com prefixo [ERRO]
// Só imprime se o nível de debug for DEBUG_ERRO ou maior
//
// Exemplo de saída: [ERRO] Falha ao conectar ao broker
// -------------------------------------------------------
void debugErro(const String& mensagem);

// -------------------------------------------------------
// Exibe uma mensagem informativa no serial com prefixo [INFO]
// Só imprime se o nível de debug for DEBUG_TUDO
//
// Exemplo de saída: [INFO] Temperatura lida: 23.5°C
// -------------------------------------------------------
void debugInfo(const String& mensagem);

// -------------------------------------------------------
// Versões sem quebra de linha no final (\n)
// Úteis para montar mensagens progressivas em uma só linha
//
// Exemplo: debugInfoSemLinha("Conectando");
//          debugInfoSemLinha(".....");
//          debugInfo(" OK!");
// Saída:   [INFO] Conectando..... OK!
// -------------------------------------------------------
void debugErroSemLinha(const String& mensagem);
void debugInfoSemLinha(const String& mensagem);

// -------------------------------------------------------
// Retorna o nível de debug atualmente ativo em tempo
// de execução (pode diferir do nível inicial se o pino
// de habilitação estiver em LOW)
// -------------------------------------------------------
int obterNivelDebugAtual();

#endif