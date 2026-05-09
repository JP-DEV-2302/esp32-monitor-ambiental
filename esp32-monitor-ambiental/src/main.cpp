/*
 * ============================================================
 * main.cpp
 * ------------------------------------------------------------
 * Ponto de entrada do firmware. Orquestra a inicialização
 * de todos os módulos e o loop principal do sistema.
 *
 * Responsabilidades:
 *   - Inicializar WiFi, MQTT, LED RGB, Lâmpada e Sensores
 *   - Receber mensagens MQTT e roteá-las para os handlers
 *   - Controlar o LED RGB NeoPixel via JSON
 *   - Controlar a lâmpada física via JSON
 *
 * Módulos utilizados:
 *   WiFiManager  → conexão e reconexão WiFi
 *   MqttManager  → publicação e recebimento de mensagens
 *   DebugManager → logs seriais por nível de verbosidade
 *   Componentes  → DHT22, buzzer e LCD
 * ============================================================
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <LED.h>
#include <DHT.h>
#include <DHT_U.h>

#include "WiFiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"
#include "Componentes.h"

// -------------------------------------------------------
// Configuração do LED RGB NeoPixel
// GPIO 48 = LED RGB integrado no ESP32-S3
// -------------------------------------------------------
const int PINO_LED_RGB    = 48;
const int QUANTIDADE_LEDS = 1;

// -------------------------------------------------------
// Configuração da lâmpada física
// Controlada pela classe Led (biblioteca LED.h)
// -------------------------------------------------------
const int PINO_LAMPADA = 40;
Led lampada(PINO_LAMPADA);

// -------------------------------------------------------
// Objeto NeoPixel para controle do LED RGB integrado
// Protocolo: GRB + 800 KHz (padrão WS2812B)
// -------------------------------------------------------
Adafruit_NeoPixel ledRGB(QUANTIDADE_LEDS, PINO_LED_RGB, NEO_GRB + NEO_KHZ800);

// -------------------------------------------------------
// Protótipos das funções definidas neste arquivo
// -------------------------------------------------------
void tratarMensagemRecebida(const char* topico, const String& mensagem);
void configurarLEDRGB();
void alterarCorLEDRGB(int vermelho, int verde, int azul);
void tratarJsonLEDRGB(const String& mensagem);

// -------------------------------------------------------
// Inicializa todos os módulos do sistema na sequência correta:
//   1. Debug serial (primeiro, para capturar todos os logs)
//   2. WiFi (necessário antes do MQTT)
//   3. MQTT (configuração do cliente)
//   4. Callback de mensagens (antes de conectar ao broker)
//   5. Conexão ao broker e assinatura de tópicos
//   6. LED RGB
//   7. Componentes físicos (DHT, LCD, Buzzer)
// -------------------------------------------------------
void setup()
{
    // Passo 1 — Debug serial (deve ser o primeiro)
    Serial.begin(9600);
    configurarDebug();
    debugInfo("=== INICIANDO SISTEMA ===");

    // Passo 2 — Conexão WiFi (pré-requisito para MQTT)
    debugInfo("Passo 2: Conectando ao WiFi...");
    conectarWiFi();

    // Passo 3 — Configuração do cliente MQTT (sem conectar ainda)
    debugInfo("Passo 3: Configurando MQTT...");
    configurarMQTT();

    // Passo 4 — Registra o callback ANTES de conectar ao broker
    // Garante que nenhuma mensagem inicial seja perdida
    debugInfo("Passo 4: Registrando callback de mensagens...");
    registrarCallbackMensagem(tratarMensagemRecebida);

    // Passo 5 — Conecta ao broker e assina os tópicos configurados
    debugInfo("Passo 5: Conectando ao broker MQTT...");
    conectarMQTT();

    // Passo 6 — Inicializa o LED RGB NeoPixel
    debugInfo("Passo 6: Configurando LED RGB...");
    configurarLEDRGB();

    // Passo 7 — Inicializa DHT22, LCD e buzzer
    debugInfo("Passo 7: Inicializando componentes fisicos...");
    setupComponentes();

    debugInfo("=== SISTEMA PRONTO ===");
}

// -------------------------------------------------------
// Loop principal: mantém conexões ativas e processa dados.
//
// Ordem importa:
//   1. WiFi  — base para tudo
//   2. MQTT  — depende do WiFi
//   3. loopMQTT — processa mensagens recebidas
//   4. Sensores — leitura e publicação periódica
// -------------------------------------------------------
void loop()
{
    // Passo 1 — Garante que o WiFi permanece conectado
    garantirWiFiConectado();

    // Passo 2 — Garante que o MQTT permanece conectado
    garantirMQTTConectado();

    // Passo 3 — Processa mensagens recebidas pelo broker
    loopMQTT();

    // Passo 4 — Leitura periódica do DHT22 e atualização do LCD
    verificarTemperaturaEUmidade();
}

// -------------------------------------------------------
// Callback chamado automaticamente pelo MqttManager
// ao receber qualquer mensagem de tópico assinado.
//
// Roteia a mensagem para o handler correto com base
// no nome do tópico recebido.
// -------------------------------------------------------
void tratarMensagemRecebida(const char* topico, const String& mensagem)
{
    // Passo 1 — Valida o tópico antes de processar
    if (topico == nullptr)
    {
        debugErro("Topico MQTT nulo recebido. Mensagem descartada.");
        return;
    }

    debugInfo("=== Mensagem recebida ===");
    debugInfo("Topico  : " + String(topico));
    debugInfo("Payload : " + mensagem);

    // Passo 2 — Roteamento por tópico

    // Controle do LED RGB e lâmpada via JSON
    if (strcmp(topico, "senai134/dev_01/Coordenador/esp32/statusLampada") == 0)
    {
        debugInfo("Roteando para: tratarJsonLEDRGB()");
        tratarJsonLEDRGB(mensagem);
        return;
    }

    // Dados de umidade recebidos de outro dispositivo
    if (strcmp(topico, "senai134/dev_01/Coordenador/esp32/statusUmidade") == 0)
    {
        debugInfo("Umidade recebida de outro dispositivo: " + mensagem + "%");
        // TODO: implementar lógica de tratamento de umidade externa
        return;
    }

    // Dados de temperatura recebidos de outro dispositivo
    if (strcmp(topico, "senai134/dev_01/Coordenador/esp32/statusTemperatura") == 0)
    {
        debugInfo("Temperatura recebida de outro dispositivo: " + mensagem + "C");
        // TODO: implementar lógica de tratamento de temperatura externa
        return;
    }

    // Status do alarme recebido de outro dispositivo
    if (strcmp(topico, "senai134/dev_01/Coordenador/esp32/statusAlarme") == 0)
    {
        debugInfo("Status de alarme recebido: " + mensagem +
                  (mensagem == "1" ? " (ALERTA)" : " (NORMAL)"));
        // TODO: implementar sincronização de alarme entre dispositivos
        return;
    }

    // Tópico recebido que não tem handler mapeado
    debugErro("Topico nao tratado: " + String(topico));
    debugErro("Verifique se o topico deve ser adicionado ao roteamento.");
}

// -------------------------------------------------------
// Inicializa o LED RGB NeoPixel com brilho padrão e apagado.
//
// Fluxo:
//   1. Inicializa a biblioteca NeoPixel
//   2. Define o brilho máximo (0–255)
//   3. Apaga todos os pixels
//   4. Envia o comando para o hardware
// -------------------------------------------------------
void configurarLEDRGB()
{
    // Passo 1 — Inicializa a comunicação com o LED
    ledRGB.begin();
    debugInfo("NeoPixel inicializado no GPIO " + String(PINO_LED_RGB) + ".");

    // Passo 2 — Define o brilho (80/255 ≈ 31%)
    ledRGB.setBrightness(80);
    debugInfo("Brilho do LED RGB definido para 80/255.");

    // Passo 3 — Apaga todos os pixels
    ledRGB.clear();

    // Passo 4 — Envia o estado para o hardware
    ledRGB.show();
    debugInfo("LED RGB configurado e apagado. Pronto para uso.");
}

// -------------------------------------------------------
// Aplica uma cor RGB ao LED NeoPixel.
// Os valores são automaticamente limitados ao intervalo 0–255.
//
// Parâmetros:
//   vermelho → intensidade do canal vermelho (0–255)
//   verde    → intensidade do canal verde    (0–255)
//   azul     → intensidade do canal azul     (0–255)
// -------------------------------------------------------
void alterarCorLEDRGB(int vermelho, int verde, int azul)
{
    // Passo 1 — Aplica a cor com proteção de limites via constrain()
    ledRGB.setPixelColor(0, ledRGB.Color(
        constrain(vermelho, 0, 255),
        constrain(verde,    0, 255),
        constrain(azul,     0, 255)
    ));

    // Passo 2 — Envia o novo estado para o hardware
    ledRGB.show();

    debugInfo("Cor do LED RGB alterada → R:" + String(vermelho) +
              " G:" + String(verde) + " B:" + String(azul));
}

// -------------------------------------------------------
// Interpreta o JSON recebido e atualiza LED RGB e/ou lâmpada.
//
// Formato esperado:
//   { "led": { "r": 255, "g": 0, "b": 0 }, "lampada": true }
//
// Fluxo:
//   1. Desserializa o JSON
//   2. Se "led" presente: valida e aplica cor no NeoPixel
//   3. Se "lampada" presente: liga ou desliga a lâmpada
// -------------------------------------------------------
void tratarJsonLEDRGB(const String& mensagem)
{
    // Passo 1 — Desserializa o JSON recebido
    JsonDocument doc;
    DeserializationError erro = deserializeJson(doc, mensagem);

    if (erro)
    {
        debugErro("Falha ao interpretar JSON: " + String(erro.c_str()));
        debugErro("Payload recebido: " + mensagem);
        return;
    }

    debugInfo("JSON deserializado com sucesso.");

    // Passo 2 — Processa o objeto "led" se presente
    if (doc["led"].is<JsonObject>())
    {
        debugInfo("Campo 'led' encontrado no JSON. Validando campos r, g, b...");

        // Valida que os três canais estão presentes e são inteiros
        if (!doc["led"]["r"].is<int>() || !doc["led"]["g"].is<int>() || !doc["led"]["b"].is<int>())
        {
            debugErro("JSON invalido para LED. Campos esperados: led.r, led.g, led.b (inteiros).");
            debugErro("JSON recebido: " + mensagem);
            return;
        }

        // Aplica a cor no NeoPixel
        alterarCorLEDRGB(
            doc["led"]["r"].as<int>(),
            doc["led"]["g"].as<int>(),
            doc["led"]["b"].as<int>()
        );
    }
    else
    {
        debugInfo("Campo 'led' ausente no JSON. LED RGB nao alterado.");
    }

    // Passo 3 — Processa o campo "lampada" se presente
    if (doc["lampada"].is<bool>())
    {
        bool estado = doc["lampada"].as<bool>();
        estado ? lampada.acender() : lampada.apagar();
        debugInfo("Lampada: " + String(estado ? "LIGADA" : "DESLIGADA") +
                  " (GPIO " + String(PINO_LAMPADA) + ")");
    }
    else
    {
        debugInfo("Campo 'lampada' ausente no JSON. Estado da lampada nao alterado.");
    }
}