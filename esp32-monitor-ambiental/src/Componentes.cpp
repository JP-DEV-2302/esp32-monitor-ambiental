/*
 * ============================================================
 * Componentes.cpp
 * ------------------------------------------------------------
 * Implementa o controle dos componentes físicos:
 *   - Sensor DHT22 (temperatura e umidade)
 *   - Buzzer (alarme sonoro não-bloqueante)
 *   - Display LCD 16×2 via I2C
 *
 * A lógica de alarme usa millis() em vez de delay(),
 * garantindo que o loop principal nunca seja bloqueado.
 * Publicações de dados são feitas via MqttManager.
 * ============================================================
 */

#include "Componentes.h"

// -------------------------------------------------------
// Instâncias dos componentes físicos
// (declaradas como extern em Componentes.h para
//  acesso externo quando necessário)
// -------------------------------------------------------
int               buzzer = BUZZER_PIN;     // Pino do buzzer (definido em Componentes.h)
DHT               dht(DHTPIN, DHTTYPE);    // Sensor de temperatura e umidade
LiquidCrystal_I2C lcd(0x27, 16, 2);       // LCD 16 colunas × 2 linhas, endereço I2C 0x27
                                            // ← troque para 0x3F se o display não acender

// -------------------------------------------------------
// Configurações de temporização (não-bloqueante)
// -------------------------------------------------------
static const unsigned long INTERVALO_LEITURA_MS = 3000UL; // Intervalo entre leituras do DHT (ms)
static const unsigned long DURACAO_TOM_MS       = 500UL;  // Duração de cada tom do alarme (ms)

// -------------------------------------------------------
// Estado interno do alarme sonoro
// Gerenciado de forma não-bloqueante via millis()
// -------------------------------------------------------
static bool          alarmeAtivo    = false; // true = alarme em toque
static int           alarmeEtapa    = 0;     // 0 = tom alto, 1 = tom baixo
static unsigned long alarmeUltimoMs = 0;     // Marca de tempo do último tom tocado

// Frequências do alarme alternado (Hz)
static const int FREQ_ALTA  = 2000; // Tom agudo
static const int FREQ_BAIXA = 800;  // Tom grave

// -------------------------------------------------------
// Inicializa todos os componentes físicos do sistema.
//
// Fluxo:
//   1. Configura o pino do buzzer como saída digital
//   2. Inicializa o sensor DHT22
//   3. Inicializa o LCD e ativa o backlight
//   4. Exibe mensagem de calibração no LCD
//   5. Aguarda 5 segundos para estabilização do DHT22
//      (tempo mínimo recomendado pelo datasheet)
//   6. Limpa o LCD para uso normal
//
// Deve ser chamada UMA VEZ no setup(), após configurarMQTT().
// -------------------------------------------------------
void setupComponentes()
{
    // Passo 1 — Configura o buzzer como saída digital
    pinMode(buzzer, OUTPUT);
    debugInfo("Buzzer configurado no GPIO " + String(buzzer) + ".");

    // Passo 2 — Inicializa o sensor DHT22
    dht.begin();
    debugInfo("Sensor DHT" + String(DHTTYPE) +
              " inicializado no GPIO " + String(DHTPIN) + ".");

    // Passo 3 — Inicializa o LCD e liga o backlight
    lcd.init();
    lcd.backlight();
    debugInfo("LCD inicializado (I2C 0x27, 16x2). Backlight ativado.");

    // Passo 4 — Exibe mensagem visual de calibração no LCD
    lcd.setCursor(0, 0);
    lcd.print("Calibrando...");
    debugInfo("Aguardando calibracao do DHT22 (5 segundos)...");

    // Passo 5 — Aguarda estabilização do DHT22 com feedback serial
    debugInfoSemLinha("[INFO] Progresso");
    for (int i = 0; i < 5; i++)
    {
        delay(1000);
        debugInfoSemLinha(".");
    }
    debugInfoSemLinha("\n\r");

    // Passo 6 — Calibração concluída, prepara LCD para exibição normal
    debugInfo("Calibracao concluida. Sensores prontos.");
    debugInfo("Sistema liberado para operacao.");

    lcd.clear();
}

// -------------------------------------------------------
// Gerencia o alarme sonoro de forma NÃO-BLOQUEANTE.
//
// Alterna entre tom alto (FREQ_ALTA) e tom baixo (FREQ_BAIXA)
// a cada DURACAO_TOM_MS milissegundos, sem usar delay().
//
// Deve ser chamada a cada iteração do loop() via
// verificarTemperaturaEUmidade().
// -------------------------------------------------------
static void atualizarAlarme()
{
    // Se o alarme não está ativo, silencia o buzzer e retorna
    if (!alarmeAtivo)
    {
        noTone(buzzer);
        return;
    }

    unsigned long agora = millis();

    // Verifica se já passou o tempo suficiente para trocar o tom
    if (agora - alarmeUltimoMs < DURACAO_TOM_MS) return;

    // Atualiza a marca de tempo e alterna entre etapa 0 e 1
    alarmeUltimoMs = agora;
    alarmeEtapa    = (alarmeEtapa + 1) % 2;

    // Toca o tom correspondente à etapa atual
    int frequencia = (alarmeEtapa == 0) ? FREQ_ALTA : FREQ_BAIXA;
    tone(buzzer, frequencia);
}

// -------------------------------------------------------
// Leitura periódica do DHT22, publicação MQTT e
// atualização do LCD. Gerencia alarme por faixa segura.
//
// Fluxo de cada ciclo (a cada INTERVALO_LEITURA_MS):
//   1. Mantém o alarme atualizado (não-bloqueante)
//   2. Aguarda o intervalo de leitura com millis()
//   3. Lê temperatura e umidade do DHT22
//   4. Trata falhas de leitura (com contador de falhas)
//   5. Publica temperatura e umidade via MQTT
//   6. Verifica se os valores estão nas faixas seguras
//   7. Ativa/desativa alarme somente na TRANSIÇÃO de estado
//   8. Atualiza o display LCD com os valores atuais
//
// Deve ser chamada A CADA iteração do loop().
// -------------------------------------------------------
void verificarTemperaturaEUmidade()
{
    // Guarda o tempo da última leitura entre chamadas
    static unsigned long ultimaLeitura = 0;
    unsigned long agora = millis();

    // Passo 1 — Mantém o alarme sonoro atualizado
    // (independente do intervalo de leitura do sensor)
    atualizarAlarme();

    // Passo 2 — Respeita o intervalo entre leituras (não-bloqueante)
    if (agora - ultimaLeitura < INTERVALO_LEITURA_MS) return;
    ultimaLeitura = agora;

    // --- Variáveis persistentes entre leituras (static) ---
    static float ultimaTemp         = NAN; // Último valor válido de temperatura
    static float ultimaUmid         = NAN; // Último valor válido de umidade
    static int   falhasConsecutivas = 0;   // Contador de falhas de leitura

    // Passo 3 — Leitura do sensor DHT22
    float temperatura = dht.readTemperature();
    float umidade     = dht.readHumidity();

    // Passo 4 — Trata falhas de leitura (NaN = sensor não respondeu)
    if (isnan(temperatura) || isnan(umidade))
    {
        falhasConsecutivas++;

        // Loga somente nas primeiras MAX_FALHAS_DHT ocorrências
        // para evitar poluição excessiva no serial
        if (falhasConsecutivas <= MAX_FALHAS_DHT)
        {
            debugErro("Falha na leitura do DHT22. ("
                + String(falhasConsecutivas) + "x consecutiva)");
        }
        else if (falhasConsecutivas == MAX_FALHAS_DHT + 1)
        {
            debugErro("Limite de logs de falha atingido. "
                      "Proximas falhas serao silenciosas.");
        }

        // Exibe o último valor válido no LCD com indicador visual de erro
        if (!isnan(ultimaTemp))
        {
            debugInfo("Exibindo ultimo valor valido no LCD (com indicador de erro).");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("T:" + String(ultimaTemp, 1) + "C ?ERRO");
            lcd.setCursor(0, 1);
            lcd.print("U:" + String(ultimaUmid, 1) + "% ?ERRO");
        }
        else
        {
            debugErro("Nenhum valor valido anterior disponivel para exibir no LCD.");
        }

        return; // Interrompe o ciclo — não publica dados inválidos
    }

    // Passo 4b — Leitura bem-sucedida: loga recuperação se havia falhas
    if (falhasConsecutivas > 0)
    {
        debugInfo("DHT22 recuperado apos " + String(falhasConsecutivas) + " falha(s).");
    }

    // Zera o contador e salva os valores válidos para uso futuro
    falhasConsecutivas = 0;
    ultimaTemp         = temperatura;
    ultimaUmid         = umidade;

    debugInfo("Leitura OK → Temp: " + String(temperatura, 1) +
              "°C | Umid: " + String(umidade, 1) + "%");

    // Passo 5 — Publica temperatura e umidade via MQTT
    char tempStr[10], umidStr[10];
    dtostrf(temperatura, 4, 1, tempStr);
    dtostrf(umidade,     4, 1, umidStr);

    debugInfo("Publicando temperatura no topico indice " +
              String(INDICE_TOPICO_TEMPERATURA) + ": " + String(tempStr));
    publicarMensagemNoTopico(INDICE_TOPICO_TEMPERATURA, tempStr);

    debugInfo("Publicando umidade no topico indice " +
              String(INDICE_TOPICO_UMIDADE) + ": " + String(umidStr));
    publicarMensagemNoTopico(INDICE_TOPICO_UMIDADE, umidStr);

    // Passo 6 — Verifica se os valores estão dentro das faixas seguras
    bool tempForaFaixa = (temperatura < TEMP_MIN || temperatura > TEMP_MAX);
    bool umidForaFaixa = (umidade     < UMID_MIN || umidade     > UMID_MAX);
    bool foraFaixa     = tempForaFaixa || umidForaFaixa;

    // Passo 7 — Aciona/desliga alarme somente na TRANSIÇÃO de estado
    // Evita publicar "1" ou "0" repetidamente a cada leitura
    if (foraFaixa && !alarmeAtivo)
    {
        // Transição: normal → alarme
        alarmeAtivo    = true;
        alarmeEtapa    = 0;
        alarmeUltimoMs = 0; // Força início imediato do tom

        debugErro(">>> ALERTA: valores FORA da faixa segura! <<<");
        if (tempForaFaixa)
            debugErro("Temperatura: " + String(temperatura, 1) +
                      "C (faixa segura: " + String(TEMP_MIN) + "-" + String(TEMP_MAX) + "C)");
        if (umidForaFaixa)
            debugErro("Umidade    : " + String(umidade, 1) +
                      "% (faixa segura: " + String(UMID_MIN) + "-" + String(UMID_MAX) + "%)");

        debugInfo("Publicando alarme=1 (estado de alerta).");
        publicarMensagemNoTopico(INDICE_TOPICO_ALARME, "1");
    }
    else if (!foraFaixa && alarmeAtivo)
    {
        // Transição: alarme → normal
        alarmeAtivo = false;
        noTone(buzzer);

        debugInfo("Valores normalizados. Alarme DESATIVADO.");
        debugInfo("Publicando alarme=0 (estado normal).");
        publicarMensagemNoTopico(INDICE_TOPICO_ALARME, "0");
    }

    // Passo 8 — Atualiza o display LCD com os valores atuais
    lcd.clear();

    // Linha 0: temperatura com indicador visual se fora da faixa
    lcd.setCursor(0, 0);
    lcd.print("T:" + String(temperatura, 1) + "C");
    if (tempForaFaixa)
    {
        lcd.print(" ALERTA!");
        debugInfo("LCD linha 0: temperatura com ALERTA.");
    }

    // Linha 1: umidade com indicador visual se fora da faixa
    lcd.setCursor(0, 1);
    lcd.print("U:" + String(umidade, 1) + "%");
    if (umidForaFaixa)
    {
        lcd.print(" ALERTA!");
        debugInfo("LCD linha 1: umidade com ALERTA.");
    }

    debugInfo("LCD atualizado.");
}