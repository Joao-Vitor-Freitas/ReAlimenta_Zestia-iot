#pragma once
#include <Arduino.h>

// ---------------- Pinos (ESP32 DevKit C) ----------------
constexpr uint8_t PIN_DHT       = 15;  // sensor DHT22 (digital)
constexpr uint8_t PIN_I2C_SDA   = 21;  // display OLED SSD1306
constexpr uint8_t PIN_I2C_SCL   = 22;
constexpr uint8_t PIN_BTN_MODE  = 18;  // alterna o modo do cooler (AUTO/LIGADO/DESLIGADO)
constexpr uint8_t PIN_BTN_PAGE  = 19;  // alterna a página do display
constexpr uint8_t PIN_BTN_FAULT = 4;   // simula falha de rede (somente demonstração)
constexpr uint8_t PIN_LED_OK    = 25;
constexpr uint8_t PIN_LED_WARN  = 26;
constexpr uint8_t PIN_LED_CRIT  = 27;
constexpr uint8_t PIN_COOLER    = 33;  // atuador (LED azul no simulador; relé no hardware real)
constexpr uint8_t PIN_BUZZER    = 13;

// ---------------- Tempos ----------------
constexpr uint32_t SAMPLE_PERIOD_MS  = 2000;   // amostragem fixa (mínimo do DHT22 = 2 s)
constexpr uint32_t PUBLISH_PERIOD_MS = 20000;  // ThingSpeak aceita 1 envio a cada 15 s por canal
constexpr uint32_t DISPLAY_PERIOD_MS = 250;
constexpr uint32_t DEBOUNCE_MS       = 50;
constexpr uint32_t WDT_TIMEOUT_S     = 10;
constexpr uint32_t WIFI_RETRY_MS     = 8000;
constexpr uint32_t MQTT_RETRY_MS     = 5000;
constexpr uint32_t ALARM_BLINK_MS    = 400;
constexpr uint8_t  SENSOR_FAIL_SAMPLES = 3;    // leituras inválidas seguidas até declarar falha

// ---------------- Regras da câmara fria (faixas do escopo do projeto) ----------------
constexpr float SETPOINT_DEFAULT_C = 4.0f;
constexpr float SETPOINT_MIN_C     = 0.0f;     // limites aceitos para o comando remoto
constexpr float SETPOINT_MAX_C     = 6.0f;
constexpr float HYSTERESIS_C       = 0.5f;
constexpr float WARN_C             = 5.0f;     // >= 5 C: atenção
constexpr float CRIT_C             = 8.0f;     // >= 8 C: crítico

// ---------------- Display ----------------
constexpr uint8_t OLED_WIDTH  = 128;
constexpr uint8_t OLED_HEIGHT = 64;
constexpr uint8_t OLED_ADDR   = 0x3C;
