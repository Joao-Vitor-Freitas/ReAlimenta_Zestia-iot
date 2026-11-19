#include "connectivity.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "Copie include/secrets.example.h para include/secrets.h e preencha as credenciais."
#endif

static WiFiClient netClient;
static PubSubClient mqtt(netClient);
static LinkState link = LinkState::WIFI_DOWN;
static FaultSim fault = FaultSim::NONE;
static CommandHandler handler = nullptr;
static uint32_t lastWifiTry = 0, lastMqttTry = 0;
static bool wifiStarted = false;
static char topicPublish[64], topicSetpoint[80], topicMode[80];

// Recepção de configuração: ThingSpeak entrega o valor do campo como texto puro.
static void onMessage(char* topic, byte* payload, unsigned int len) {
  char buf[16];
  if (len == 0 || len >= sizeof(buf)) return;
  memcpy(buf, payload, len);
  buf[len] = 0;
  char* end = nullptr;
  float v = strtof(buf, &end);
  if (end == buf || !handler) return;
  RemoteCommand c;
  if (strcmp(topic, topicSetpoint) == 0) c.type = CommandType::SET_SETPOINT;
  else if (strcmp(topic, topicMode) == 0) c.type = CommandType::SET_MODE;
  else return;
  c.value = v;
  handler(c);
}

void connectivityBegin(CommandHandler h) {
  handler = h;
  snprintf(topicPublish, sizeof(topicPublish), "channels/%s/publish", TS_CH_TELEMETRY);
  snprintf(topicSetpoint, sizeof(topicSetpoint), "channels/%s/subscribe/fields/field1", TS_CH_COMMANDS);
  snprintf(topicMode, sizeof(topicMode), "channels/%s/subscribe/fields/field2", TS_CH_COMMANDS);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(false);  // a reconexão é controlada pela máquina de estados
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMessage);
  mqtt.setSocketTimeout(3);  // limita o bloqueio do connect() (watchdog = 10 s)
  mqtt.setKeepAlive(30);
}

void connectivityLoop(uint32_t now) {
  if (fault == FaultSim::WIFI_BLOCKED) {  // falha simulada: derruba o WiFi e não reconecta
    if (mqtt.connected()) mqtt.disconnect();
    if (wifiStarted) { WiFi.disconnect(); wifiStarted = false; }
    link = LinkState::WIFI_DOWN;
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    link = LinkState::WIFI_DOWN;
    if (!wifiStarted || now - lastWifiTry >= WIFI_RETRY_MS) {
      lastWifiTry = now;
      wifiStarted = true;
      WiFi.begin(WIFI_SSID, WIFI_PASS, WIFI_CHANNEL);  // retorna na hora; o status muda depois
    }
    return;
  }
  if (fault == FaultSim::MQTT_BLOCKED) {  // falha simulada: WiFi ok, broker inacessível
    if (mqtt.connected()) mqtt.disconnect();
    link = LinkState::MQTT_DOWN;
    return;
  }
  if (!mqtt.connected()) {
    link = LinkState::MQTT_DOWN;
    if (now - lastMqttTry >= MQTT_RETRY_MS) {
      lastMqttTry = now;
      if (mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
        mqtt.subscribe(topicSetpoint);
        mqtt.subscribe(topicMode);
        link = LinkState::ONLINE;
      }
    }
    return;
  }
  mqtt.loop();
  link = mqtt.connected() ? LinkState::ONLINE : LinkState::MQTT_DOWN;
}

LinkState connectivityState() { return link; }
FaultSim connectivityFault() { return fault; }
int connectivityRssi() { return WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0; }

void connectivityCycleFault() {
  fault = fault == FaultSim::NONE ? FaultSim::MQTT_BLOCKED
        : fault == FaultSim::MQTT_BLOCKED ? FaultSim::WIFI_BLOCKED : FaultSim::NONE;
  lastWifiTry = 0;
  lastMqttTry = 0;
}

// field1 temp média | field2 umidade média | field3 cooler | field4 setpoint | field5 modo
// field6 estado | field7 temp máxima | field8 RSSI
bool connectivityPublish(const Telemetry& t) {
  if (link != LinkState::ONLINE) return false;
  char p[200];
  int n = 0;
  if (!isnan(t.tempAvg)) n += snprintf(p + n, sizeof(p) - n, "field1=%.2f&", t.tempAvg);
  if (!isnan(t.humAvg)) n += snprintf(p + n, sizeof(p) - n, "field2=%.1f&", t.humAvg);
  n += snprintf(p + n, sizeof(p) - n, "field3=%d&field4=%.1f&field5=%d&field6=%d&", t.coolerOn ? 1 : 0, t.setpointC, (int)t.mode, (int)t.state);
  if (!isnan(t.tempMax)) n += snprintf(p + n, sizeof(p) - n, "field7=%.2f&", t.tempMax);
  snprintf(p + n, sizeof(p) - n, "field8=%d&status=MQTTPUBLISH", t.rssi);
  return mqtt.publish(topicPublish, p);
}
