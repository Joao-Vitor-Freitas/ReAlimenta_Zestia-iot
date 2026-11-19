#include "actuator.h"
#include <esp_arduino_version.h>
#include "config.h"

static ActuatorStatus st = { false, false, TempState::OK };
static bool coolerOn = false;
static uint8_t invalidCount = 0;
static bool haveTemp = false;
static float lastTemp = 0;
static bool buzzerActive = false;
static const uint8_t BUZZER_CH = 0;

static void buzzerInit() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(PIN_BUZZER, 2000, 8);
#else
  ledcSetup(BUZZER_CH, 2000, 8);
  ledcAttachPin(PIN_BUZZER, BUZZER_CH);
#endif
}
static void buzzerSet(bool on) {
  if (on == buzzerActive) return;
  buzzerActive = on;
  uint32_t f = on ? 2000 : 0;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWriteTone(PIN_BUZZER, f);
#else
  ledcWriteTone(BUZZER_CH, f);
#endif
}

void actuatorBegin() {
  const uint8_t outs[] = { PIN_LED_OK, PIN_LED_WARN, PIN_LED_CRIT, PIN_COOLER };
  for (uint8_t p : outs) { pinMode(p, OUTPUT); digitalWrite(p, LOW); }
  buzzerInit();
}

static TempState classify(float t, const Settings& c) {
  if (t >= c.critC) return TempState::CRITICAL;
  if (t >= c.warnC) return TempState::ATTENTION;
  return TempState::OK;
}

static ActuatorStatus decide(const Settings& c) {
  switch (c.mode) {
    case CoolerMode::FORCED_ON:  coolerOn = true;  break;
    case CoolerMode::FORCED_OFF: coolerOn = false; break;
    default:  // AUTO
      if (st.state == TempState::SENSOR_FAIL || st.state == TempState::CRITICAL) {
        coolerOn = true;  // falha segura: sem leitura confiável ou temperatura crítica, refrigera
      } else if (haveTemp) {
        if (lastTemp >= c.setpointC + c.hysteresisC) coolerOn = true;
        else if (lastTemp <= c.setpointC - c.hysteresisC) coolerOn = false;
      }
  }
  st.coolerOn = coolerOn;
  st.alarmOn = (st.state == TempState::CRITICAL || st.state == TempState::SENSOR_FAIL);
  digitalWrite(PIN_COOLER, coolerOn);
  digitalWrite(PIN_LED_OK, st.state == TempState::OK);
  digitalWrite(PIN_LED_WARN, st.state == TempState::ATTENTION);
  return st;
}

ActuatorStatus actuatorEvaluate(const Sample& s, const Settings& c) {
  if (s.valid) {
    invalidCount = 0;
    haveTemp = true;
    lastTemp = s.tempC;
    st.state = classify(s.tempC, c);
  } else if (++invalidCount >= SENSOR_FAIL_SAMPLES) {
    invalidCount = SENSOR_FAIL_SAMPLES;
    st.state = TempState::SENSOR_FAIL;
  }
  return decide(c);
}

ActuatorStatus actuatorReapply(const Settings& c) { return decide(c); }

void actuatorTick(uint32_t nowMs) {
  bool phase = st.alarmOn && ((nowMs / ALARM_BLINK_MS) % 2 == 0);
  digitalWrite(PIN_LED_CRIT, phase);
  buzzerSet(phase);
}
