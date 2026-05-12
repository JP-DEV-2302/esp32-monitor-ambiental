🌡️ ESP32 — MQTT Environmental Monitor
Firmware for ESP32 that monitors temperature and humidity using a DHT22 sensor, publishes data to an MQTT broker (HiveMQ Cloud with TLS), and triggers visual and audible alarms when values exceed safe ranges.

📋 Features


Temperature and humidity monitoring (DHT11 or DHT22)


MQTT publishing with TLS (HiveMQ Cloud)


Non-blocking buzzer alarm


16×2 LCD display with real-time readings


NeoPixel RGB LED control via JSON


Lamp control via JSON


Multi-level serial debug system


Automatic WiFi and MQTT reconnection



🔧 Required Hardware
ComponentDefault GPIODHT22 Sensor10Buzzer8RGB LED (NeoPixel)48Lamp (Relay/LED)40LCD I2C (0x27)SDA/SCLPhysical debug pin4

🚀 Getting Started
1. Clone the repository
git clone https://github.com/your-username/esp32-environment-monitor.gitcd esp32-environment-monitor
2. Configure credentials
cp secrets.example.cpp src/secrets.cpp
Edit src/secrets.cpp with your WiFi and MQTT credentials.
3. Build and flash
Open the project in PlatformIO or Arduino IDE and upload it to the ESP32.

📡 MQTT Topics
DirectionTopicContentPublish.../comandoTemperaturaExample: 23.5Publish.../comandoUmidadeExample: 58.2Publish.../statusAlarme1 = alert, 0 = okSubscribe.../# (wildcard)JSON or raw values
JSON format for LED and lamp control
{ "led": { "r": 255, "g": 0, "b": 0 }, "lampada": true }

⚙️ Sensor Configuration
In include/Componentes.h:
#define DHTPIN     10      // Sensor GPIO#define DHTTYPE    DHT22   // DHT11 or DHT22#define BUZZER_PIN 8       // Buzzer GPIO

📁 Project Structure
├── src/                  # Source files (.cpp)├── include/              # Header files (.h)├── secrets.example.cpp   # Credentials template└── README.md

📄 License
MIT License — feel free to use and modify this project.
