// ReAlimenta - monitor de câmara fria (ESP32 + Wokwi)
// Fluxo: tarefa de amostragem -> fila -> loop() (lógica de borda, display, MQTT). Sem delay().
#include <Arduino.h>
#include <esp_system.h>
#include "config.h"
#include "types.h"
#include "sensor.h"
#include "actuator.h"
#include "buttons.h"
#include "display_ui.h"
#include "connectivity.h"
#include "watchdog.h"

struct WindowStats { float sumT; float sumH; float maxT; uint16_t n; };

static Settings cfg = { SETPOINT_DEFAULT_C, HYSTERESIS_C, WARN_C, CRIT_C, CoolerMode::AUTO };
static ActuatorStatus act = { false, false, TempState::OK };
static Sample lastSample = { 0, 0, 0, 0, false };
static bool hasSample = false;
static WindowStats win = { 0, 0, -1000.0f, 0 };
static uint32_t lastPublishMs = 0, lastDisplayMs = 0, lastPublishOkMs = 0;
static bool everPublished = false;
static const char* resetReason = "?";

static const char* readResetReason() {
  switch (esp_reset_reason()) {
    case ESP_RST_POWERON: return "POWER";
    case ESP_RST_TASK_WDT: return "WDT";
    case ESP_RST_INT_WDT: return "WDT-I";
    case ESP_RST_PANIC: return "PANIC";
    case ESP_RST_BROWNOUT: return "BROWN";
    default: return "OUTRO";
  }
}

static void resetWindow() { win.sumT = 0; win.sumH = 0; win.maxT = -1000.0f; win.n = 0; }

static void onSample(const Sample& s) {
  lastSample = s;
  hasSample = true;
  act = actuatorEvaluate(s, cfg);
  if (s.valid) {
    win.sumT += s.tempC; win.sumH += s.humidity; win.n++;
    if (s.tempC > win.maxT) win.maxT = s.tempC;
  }
  Serial.printf("[SENS] T=%.1fC U=%.1f%% periodo=%.3fms estado=%s cooler=%s\n",
                s.tempC, s.humidity, s.periodUs / 1000.0f, toString(act.state), act.coolerOn ? "ON" : "OFF");
}

// Configuração remota (ThingSpeak, campo do canal de comandos). Valores fora do limite são recusados.
static void onRemoteCommand(const RemoteCommand& c) {
  if (c.type == CommandType::SET_SETPOINT) {
    if (c.value < SETPOINT_MIN_C || c.value > SETPOINT_MAX_C) { Serial.printf("[CMD] setpoint %.1f recusado\n", c.value); return; }
    cfg.setpointC = c.value;
    Serial.printf("[CMD] setpoint = %.1fC\n", c.value);
  } else {
    int m = (int)(c.value + 0.5f);
    if (m < 0 || m > 2) { Serial.printf("[CMD] modo %d recusado\n", m); return; }
    cfg.mode = static_cast<CoolerMode>(m);
    Serial.printf("[CMD] modo = %s\n", toString(cfg.mode));
  }
  act = actuatorReapply(cfg);
}

static void onButton(ButtonEvent ev) {
  switch (ev) {
    case ButtonEvent::MODE:  // AUTO -> LIGADO -> DESLIGADO -> AUTO
      cfg.mode = static_cast<CoolerMode>((static_cast<uint8_t>(cfg.mode) + 1) % 3);
      act = actuatorReapply(cfg);
      Serial.printf("[BTN] modo = %s\n", toString(cfg.mode));
      break;
    case ButtonEvent::PAGE: displayTogglePage(); break;
    case ButtonEvent::FAULT:
      connectivityCycleFault();
      Serial.printf("[BTN] falha simulada = %d\n", (int)connectivityFault());
      break;
  }
}

static void publishWindow(uint32_t now) {
  Telemetry t;
  t.tempAvg = win.n ? win.sumT / win.n : NAN;  // média do período: dado já processado na borda
  t.humAvg = win.n ? win.sumH / win.n : NAN;
  t.tempMax = win.n ? win.maxT : NAN;
  t.coolerOn = act.coolerOn; t.setpointC = cfg.setpointC; t.mode = cfg.mode; t.state = act.state;
  t.rssi = connectivityRssi();
  if (connectivityPublish(t)) { everPublished = true; lastPublishOkMs = now; Serial.println("[MQTT] telemetria enviada"); }
  else Serial.println("[MQTT] offline: envio descartado");
  resetWindow();  // cada envio resume somente o período corrente
}

static void renderUi() {
  UiModel m;
  m.last = lastSample; m.hasSample = hasSample; m.cfg = cfg; m.act = act;
  m.link = connectivityState(); m.fault = connectivityFault();
  m.secondsSincePublish = everPublished ? (int32_t)((millis() - lastPublishOkMs) / 1000) : -1;
  m.resetReason = resetReason;
  displayRender(m);
}

void setup() {
  Serial.begin(115200);
  resetReason = readResetReason();
  Serial.printf("\n[BOOT] ReAlimenta IoT | motivo do reset: %s\n", resetReason);
  wdtBegin();
  displayBegin();
  actuatorBegin();
  buttonsBegin();
  connectivityBegin(onRemoteCommand);
  if (!sensorBegin()) Serial.println("[ERRO] falha ao criar a tarefa do sensor");
}

void loop() {
  uint32_t now = millis();
  wdtFeed();

  Sample s;
  while (sensorReceive(s)) onSample(s);

  ButtonEvent ev;
  while (buttonsPoll(ev)) onButton(ev);

  connectivityLoop(now);

  if (now - lastPublishMs >= PUBLISH_PERIOD_MS) { lastPublishMs = now; publishWindow(now); }

  actuatorTick(now);

  if (now - lastDisplayMs >= DISPLAY_PERIOD_MS) { lastDisplayMs = now; renderUi(); }

#ifdef WDT_TEST_HANG
  if (now > 40000) { Serial.println("[TESTE] travando o loop de propósito..."); while (true) {} }
#endif
}
