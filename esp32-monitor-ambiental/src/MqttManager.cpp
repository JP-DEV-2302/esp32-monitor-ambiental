/*
 * ============================================================
 * MqttManager.cpp
 * ------------------------------------------------------------
 * Implementa toda a camada de comunicação MQTT do ESP32.
 * Responsável por:
 *   - Configurar o cliente (com ou sem TLS)
 *   - Conectar ao broker e assinar tópicos
 *   - Publicar mensagens
 *   - Receber mensagens e despachar para o callback da aplicação
 *   - Reconectar automaticamente quando necessário
 *
 * Credenciais e tópicos vêm de secrets.cpp.
 * ============================================================
 */

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include "WiFiManager.h"
#include "MqttManager.h"
#include "secrets.h"
#include "DebugManager.h"

// -------------------------------------------------------
// Clientes de rede:
//   wifiCliente      → conexão TCP simples (sem criptografia)
//   wifiClientSecure → conexão TLS (porta 8883, HiveMQ Cloud)
// Apenas um deles será usado, conforme MQTT_USAR_TLS.
// -------------------------------------------------------
WiFiClient       wifiCliente;
WiFiClientSecure wifiClientSecure;

// Cliente MQTT principal — usa um dos clientes acima internamente
PubSubClient mqttClient;

// Ponteiro para o callback registrado pela aplicação (main.cpp).
// É chamado automaticamente ao receber qualquer mensagem MQTT.
CallbackMensagemMQTT callbackDaAplicacao = nullptr;

// -------------------------------------------------------
// Registra a função de callback da aplicação.
// Deve ser chamada no setup() ANTES de conectarMQTT(),
// para garantir que mensagens não sejam perdidas.
//
// Parâmetro:
//   callback → ponteiro para a função definida em main.cpp
// -------------------------------------------------------
void registrarCallbackMensagem(CallbackMensagemMQTT callback)
{
    // Passo 1 — Armazena o ponteiro do callback
    callbackDaAplicacao = callback;

    // Passo 2 — Confirma o registro
    if (callbackDaAplicacao != nullptr)
        debugInfo("Callback da aplicacao registrado com sucesso.");
    else
        debugErro("Falha ao registrar callback: ponteiro nulo recebido.");
}

// -------------------------------------------------------
// Retorna o tópico de publicação pelo índice.
// Consulta o array TOPICOS_PUBLICAR[] de secrets.cpp.
// Loga erro se o índice estiver fora do intervalo válido.
// -------------------------------------------------------
const char* obterTopicoPublicacao(int indiceTopico)
{
    // Passo 1 — Valida o índice antes de acessar o array
    if (indiceTopico < 0 || indiceTopico >= TOTAL_TOPICOS_PUBLICAR)
    {
        debugErro("Indice de publicacao invalido: " + String(indiceTopico) +
                  " (total: " + String(TOTAL_TOPICOS_PUBLICAR) + ")");
        return "";
    }

    // Passo 2 — Retorna o tópico correspondente
    return TOPICOS_PUBLICAR[indiceTopico];
}

// -------------------------------------------------------
// Retorna o tópico de recebimento (assinatura) pelo índice.
// Consulta o array TOPICOS_RECEBER[] de secrets.cpp.
// Loga erro se o índice estiver fora do intervalo válido.
// -------------------------------------------------------
const char* obterTopicoRecebimento(int indiceTopico)
{
    // Passo 1 — Valida o índice
    if (indiceTopico < 0 || indiceTopico >= TOTAL_TOPICOS_RECEBER)
    {
        debugErro("Indice de recebimento invalido: " + String(indiceTopico) +
                  " (total: " + String(TOTAL_TOPICOS_RECEBER) + ")");
        return "";
    }

    // Passo 2 — Retorna o tópico correspondente
    return TOPICOS_RECEBER[indiceTopico];
}

// -------------------------------------------------------
// Retorna o total de tópicos configurados para recebimento.
// Usado em conectarMQTT() para iterar sobre as assinaturas.
// -------------------------------------------------------
int obterTotalTopicosRecebimento()
{
    return TOTAL_TOPICOS_RECEBER;
}

// -------------------------------------------------------
// Callback INTERNO chamado pela biblioteca PubSubClient
// ao receber qualquer mensagem de um tópico assinado.
//
// Fluxo:
//   1. Converte o payload (bytes) em String legível
//   2. Loga tópico e conteúdo
//   3. Repassa para o callback da aplicação (main.cpp)
//
// Parâmetros (definidos pela assinatura da PubSubClient):
//   topico  → nome do tópico onde a mensagem chegou
//   payload → bytes da mensagem recebida
//   tamanho → número de bytes no payload
// -------------------------------------------------------
void callbackInternoMQTT(char* topico, byte* payload, unsigned int tamanho)
{
    // Passo 1 — Converte o payload de bytes para String
    String mensagem = "";
    for (unsigned int i = 0; i < tamanho; i++)
        mensagem += (char)payload[i];

    // Passo 2 — Loga a mensagem recebida para diagnóstico
    debugInfo("--- Mensagem MQTT recebida ---");
    debugInfo("Topico  : " + String(topico));
    debugInfo("Payload : " + mensagem);
    debugInfo("Tamanho : " + String(tamanho) + " bytes");

    // Passo 3 — Repassa para a lógica da aplicação
    if (callbackDaAplicacao != nullptr)
    {
        callbackDaAplicacao(topico, mensagem);
    }
    else
    {
        // Mensagem recebida mas sem handler registrado — avisa o desenvolvedor
        debugErro("Mensagem recebida sem callback registrado. Chame registrarCallbackMensagem() no setup().");
    }
}

// -------------------------------------------------------
// Configura o cliente MQTT com broker, porta, TLS e callbacks.
//
// Fluxo:
//   1. Escolhe o cliente de rede (TLS ou simples)
//   2. Valida e aplica o certificado CA (se TLS ativo)
//   3. Configura o servidor e o callback interno
//
// Deve ser chamada UMA VEZ no setup(), após conectarWiFi().
// -------------------------------------------------------
void configurarMQTT()
{
    debugInfo("=== Configurando MQTT ===");

    if (USAR_AWS_IOT)
    {
        // Passo 1a — Modo AWS IoT (ainda não implementado)
        debugInfo("Modo: AWS IoT (pendente de implementacao).");
        // TODO: configurar certificados de cliente e chave privada para mTLS
    }
    else if (MQTT_USAR_TLS)
    {
        // Passo 1b — Modo HiveMQ Cloud com TLS
        debugInfo("Modo: MQTT com TLS (porta " + String(MQTT_PORTA) + ").");

        // Passo 2 — Valida e aplica o certificado CA
        if (strlen(MQTT_CERTIFICADO_CA) > 100)
        {
            wifiClientSecure.setCACert(MQTT_CERTIFICADO_CA);
            debugInfo("Certificado CA aplicado com sucesso.");
        }
        else
        {
            // Certificado ausente ou inválido — usa modo inseguro (apenas para testes!)
            debugErro("Certificado CA ausente ou invalido (< 100 chars).");
            debugErro("Usando setInsecure(). NAO RECOMENDADO em producao!");
            wifiClientSecure.setInsecure();
        }

        mqttClient.setClient(wifiClientSecure);
    }
    else
    {
        // Passo 1c — Modo sem TLS (broker local ou desenvolvimento)
        debugInfo("Modo: MQTT sem TLS (porta " + String(MQTT_PORTA) + ").");
        mqttClient.setClient(wifiCliente);
    }

    // Passo 3 — Configura o endereço do broker e o callback interno
    mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);
    mqttClient.setCallback(callbackInternoMQTT);

    debugInfo("Broker configurado: " + String(MQTT_BROKER));
    debugInfo("Porta: " + String(MQTT_PORTA));
    debugInfo("Client ID: " + String(MQTT_CLIENT_ID));
    debugInfo("=== MQTT configurado ===");
}

// -------------------------------------------------------
// Realiza a conexão ao broker MQTT e assina os tópicos.
//
// Fluxo:
//   1. Verifica se o WiFi está ativo (pré-requisito)
//   2. Tenta conectar até maxTentativas vezes
//   3. Autentica com usuário/senha se configurados
//   4. Assina todos os tópicos de TOPICOS_RECEBER[]
//   5. Loga resultado de cada etapa
// -------------------------------------------------------
void conectarMQTT()
{
    // Passo 1 — Pré-requisito: WiFi deve estar conectado
    if (!wifiEstaConectado())
    {
        debugErro("WiFi desconectado. Conexao MQTT abortada.");
        return;
    }

    const int maxTentativas = 5;
    int tentativas = 0;

    debugInfo("=== Conectando ao broker MQTT ===");

    // Passo 2 — Loop de tentativas de conexão
    while (!mqttClient.connected() && tentativas < maxTentativas)
    {
        tentativas++;
        debugInfo("Tentativa " + String(tentativas) + " de " + String(maxTentativas) + "...");

        bool conectado = false;

        if (USAR_AWS_IOT)
        {
            // TODO: implementar conexão para AWS IoT (mTLS sem usuário/senha)
            debugErro("Conexao AWS IoT nao implementada.");
        }
        else
        {
            // Passo 3 — Conecta com ou sem autenticação
            if (strlen(MQTT_USUARIO) > 0)
            {
                debugInfo("Autenticando como: " + String(MQTT_USUARIO));
                conectado = mqttClient.connect(MQTT_CLIENT_ID, MQTT_USUARIO, MQTT_SENHA);
            }
            else
            {
                debugInfo("Conectando sem autenticacao.");
                conectado = mqttClient.connect(MQTT_CLIENT_ID);
            }
        }

        if (conectado)
        {
            debugInfo("Broker MQTT conectado com sucesso!");

            // Passo 4 — Assina todos os tópicos configurados em secrets.cpp
            int total = obterTotalTopicosRecebimento();
            debugInfo("Assinando " + String(total) + " topico(s)...");

            for (int i = 0; i < total; i++)
            {
                const char* topico = obterTopicoRecebimento(i);

                if (strlen(topico) == 0)
                {
                    debugErro("Topico [" + String(i) + "] vazio. Pulando.");
                    continue;
                }

                if (mqttClient.subscribe(topico))
                    debugInfo("[" + String(i) + "] Inscrito em: " + String(topico));
                else
                    debugErro("[" + String(i) + "] Falha ao se inscrever em: " + String(topico));
            }

            debugInfo("=== MQTT pronto para uso ===");
        }
        else
        {
            // Passo 5 — Falha: loga o código de erro da biblioteca
            int codigoErro = mqttClient.state();
            debugErro("Falha na conexao. Codigo de estado: " + String(codigoErro));
            debugErro("Referencia: -4=timeout, -3=servidor negou, -2=falha rede, "
                      "-1=desconectado, 1=protocolo invalido, 2=id rejeitado, "
                      "3=indisponivel, 4=credenciais invalidas, 5=nao autorizado");

            if (tentativas < maxTentativas)
            {
                debugInfo("Aguardando 2s para nova tentativa...");
                delay(2000);
            }
        }
    }

    // Resultado final após todas as tentativas
    if (!mqttClient.connected())
        debugErro("Nao foi possivel conectar ao broker apos " +
                  String(maxTentativas) + " tentativas. Verifique rede e credenciais.");
}

// -------------------------------------------------------
// Verifica a conexão MQTT no loop e reconecta se necessário.
// Deve ser chamada A CADA iteração do loop().
// -------------------------------------------------------
void garantirMQTTConectado()
{
    // Pré-requisito silencioso: WiFi deve estar ativo
    if (!wifiEstaConectado())
    {
        debugErro("WiFi desconectado. Reconexao MQTT ignorada ate WiFi voltar.");
        return;
    }

    // Verifica e reconecta somente se necessário (sem spam de log quando estável)
    if (!mqttClient.connected())
    {
        debugErro("MQTT desconectado. Iniciando reconexao...");
        conectarMQTT();

        if (mqttClient.connected())
            debugInfo("MQTT reconectado com sucesso.");
        else
            debugErro("Reconexao MQTT falhou. Proxima tentativa no proximo loop.");
    }
}

// -------------------------------------------------------
// Processa as mensagens MQTT recebidas pelo broker.
// Internamente dispara callbackInternoMQTT() quando há
// mensagem nova, que por sua vez chama callbackDaAplicacao.
//
// Deve ser chamada A CADA iteração do loop().
// -------------------------------------------------------
void loopMQTT()
{
    mqttClient.loop();
}

// -------------------------------------------------------
// Publica uma mensagem em um tópico MQTT pelo nome completo.
//
// Fluxo:
//   1. Verifica conexão ativa
//   2. Publica e loga resultado
// -------------------------------------------------------
void publicarMensagem(const char* topico, const char* mensagem)
{
    // Passo 1 — Verifica se o MQTT está conectado antes de publicar
    if (!mqttClient.connected())
    {
        debugErro("MQTT desconectado. Publicacao abortada.");
        debugErro("Topico alvo: " + String(topico));
        return;
    }

    // Passo 2 — Publica e loga o resultado
    if (mqttClient.publish(topico, mensagem))
    {
        debugInfo("Publicado → [" + String(topico) + "]: " + String(mensagem));
    }
    else
    {
        debugErro("Falha ao publicar em: " + String(topico));
        debugErro("Verifique tamanho do payload e estado da conexao.");
    }
}

// -------------------------------------------------------
// Publica uma mensagem usando o ÍNDICE do tópico em
// TOPICOS_PUBLICAR[] (definido em secrets.cpp).
// Evita hardcode de strings de tópico espalhadas pelo código.
//
// Parâmetros:
//   indiceTopico → posição no array TOPICOS_PUBLICAR[]
//   mensagem     → conteúdo a ser publicado
// -------------------------------------------------------
void publicarMensagemNoTopico(int indiceTopico, const char* mensagem)
{
    // Passo 1 — Obtém o tópico pelo índice (já valida internamente)
    const char* topico = obterTopicoPublicacao(indiceTopico);

    // Passo 2 — Verifica se o tópico é válido antes de publicar
    if (strlen(topico) == 0)
    {
        debugErro("Publicacao cancelada. Indice invalido: " + String(indiceTopico));
        return;
    }

    // Passo 3 — Delega para a função de publicação direta
    publicarMensagem(topico, mensagem);
}

// -------------------------------------------------------
// Retorna true se o cliente MQTT estiver conectado ao broker.
// -------------------------------------------------------
bool mqttEstaConectado()
{
    return mqttClient.connected();
}