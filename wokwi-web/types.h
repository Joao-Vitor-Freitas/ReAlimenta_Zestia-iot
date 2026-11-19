#pragma once
#include <Arduino.h>

enum class TempState : uint8_t { OK = 0, ATTENTION = 1, CRITICAL = 2, SENSOR_FAIL = 3 };
enum class CoolerMode : uint8_t { AUTO = 0, FORCED_ON = 1, FORCED_OFF = 2 };
enum class LinkState : uint8_t { WIFI_DOWN, MQTT_DOWN, ONLINE };
enum class FaultSim : uint8_t { NONE, MQTT_BLOCKED, WIFI_BLOCKED };  // simulação de falhas de rede
enum class ButtonEvent : uint8_t { MODE, PAGE, FAULT };
enum class CommandType : uint8_t { SET_SETPOINT, SET_MODE };

// Uma leitura do sensor (produzida pela tarefa de amostragem)
struct Sample {
  float tempC;
  float humidity;
  uint32_t timestampMs;  // instante da amostra (esp_timer)
  uint32_t periodUs;     // intervalo real desde a amostra anterior (mede o jitter)
  bool valid;
};

// Parâmetros de operação (alteráveis por MQTT e pelo botão)
struct Settings {
  float setpointC;
  float hysteresisC;
  float warnC;
  float critC;
  CoolerMode mode;
};

struct ActuatorStatus {
  bool coolerOn;
  bool alarmOn;
  TempState state;
};

// Resumo enviado à nuvem a cada período de publicação
struct Telemetry {
  float tempAvg;
  float tempMax;
  float humAvg;
  bool coolerOn;
  float setpointC;
  CoolerMode mode;
  TempState state;
  int rssi;
};

struct RemoteCommand {
  CommandType type;
  float value;
};

// Tudo o que o display precisa para desenhar
struct UiModel {
  Sample last;
  bool hasSample;
  Settings cfg;
  ActuatorStatus act;
  LinkState link;
  FaultSim fault;
  int32_t secondsSincePublish;  // -1 = ainda não enviou
  const char* resetReason;
};

inline const char* toString(TempState s) {
  switch (s) {
    case TempState::OK: return "NORMAL";
    case TempState::ATTENTION: return "ATENCAO";
    case TempState::CRITICAL: return "CRITICO";
    default: return "SENSOR!";
  }
}
inline const char* toString(CoolerMode m) {
  switch (m) {
    case CoolerMode::AUTO: return "AUTO";
    case CoolerMode::FORCED_ON: return "LIGADO";
    default: return "DESLIG.";
  }
}
