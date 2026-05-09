# 🌡️ ESP32 — Monitor Ambiental MQTT

Firmware para ESP32 que monitora temperatura e umidade via DHT22,
publica dados em broker MQTT (HiveMQ Cloud com TLS) e aciona alarme
sonoro e visual quando os valores saem das faixas seguras.

## 📋 Funcionalidades

- Leitura de temperatura e umidade (DHT11 ou DHT22)
- Publicação via MQTT com TLS (HiveMQ Cloud)
- Alarme sonoro não-bloqueante (buzzer)
- Display LCD 16×2 com valores em tempo real
- Controle de LED RGB NeoPixel via JSON
- Controle de lâmpada via JSON
- Sistema de debug por níveis via serial
- Reconexão automática de WiFi e MQTT

## 🔧 Hardware necessário

| Componente         | GPIO padrão |
|--------------------|-------------|
| Sensor DHT22       | 10          |
| Buzzer             | 8           |
| LED RGB (NeoPixel) | 48          |
| Lâmpada (relé/LED) | 40          |
| LCD I2C (0x27)     | SDA/SCL     |
| Pino debug físico  | 4           |

## 🚀 Como usar

### 1. Clone o repositório
\`\`\`bash
git clone https://github.com/seu-usuario/esp32-monitor-ambiental.git
cd esp32-monitor-ambiental
\`\`\`

### 2. Configure as credenciais
\`\`\`bash
cp secrets.example.cpp src/secrets.cpp
\`\`\`
Edite `src/secrets.cpp` com suas credenciais WiFi e MQTT.

### 3. Compile e grave
Abra no PlatformIO ou Arduino IDE e grave no ESP32.

## 📡 Tópicos MQTT

| Direção  | Tópico                                        | Conteúdo              |
|----------|-----------------------------------------------|-----------------------|
| Publica  | .../comandoTemperatura                        | Ex: `23.5`            |
| Publica  | .../comandoUmidade                            | Ex: `58.2`            |
| Publica  | .../statusAlarme                              | `1` = alerta, `0` = ok|
| Escuta   | .../# (wildcard)                              | JSON ou valores       |

### Formato JSON para controle do LED e lâmpada
\`\`\`json
{ "led": { "r": 255, "g": 0, "b": 0 }, "lampada": true }
\`\`\`

## ⚙️ Configuração do sensor

Em `include/Componentes.h`:
\`\`\`cpp
#define DHTPIN    10      // GPIO do sensor
#define DHTTYPE   DHT22   // DHT11 ou DHT22
#define BUZZER_PIN 8      // GPIO do buzzer
\`\`\`

## 📁 Estrutura do projeto

\`\`\`
├── src/                  # Implementações (.cpp)
├── include/              # Cabeçalhos (.h)
├── secrets.example.cpp   # Modelo de credenciais
└── README.md
\`\`\`

## 📄 Licença

MIT License — sinta-se livre para usar e modificar.