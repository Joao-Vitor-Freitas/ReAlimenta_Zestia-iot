#include "display_ui.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

static Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
static uint8_t page = 0;
static bool ready = false;

bool displayBegin() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  ready = oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  if (ready) {
    oled.setTextColor(SSD1306_WHITE);
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.print("ReAlimenta - iniciando");
    oled.display();
  }
  return ready;
}

void displayTogglePage() { page = (page + 1) % 2; }

static void line(int y, const char* text) { oled.setCursor(0, y); oled.print(text); }

static void drawMain(const UiModel& m) {
  char buf[32];
  oled.setTextSize(1);
  line(0, "ReAlimenta");
  const char* st = toString(m.act.state);
  oled.setCursor(OLED_WIDTH - 6 * strlen(st), 0);
  oled.print(st);

  oled.setTextSize(3);  // valor atual da medida
  if (m.hasSample && m.last.valid) snprintf(buf, sizeof(buf), "%.1fC", m.last.tempC);
  else snprintf(buf, sizeof(buf), "--.-C");
  line(12, buf);

  oled.setTextSize(1);
  if (m.hasSample && m.last.valid) snprintf(buf, sizeof(buf), "Umid:%.0f%%  Cooler:%s", m.last.humidity, m.act.coolerOn ? "ON" : "OFF");
  else snprintf(buf, sizeof(buf), "Umid:--%%  Cooler:%s", m.act.coolerOn ? "ON" : "OFF");
  line(40, buf);

  // status de conexão
  snprintf(buf, sizeof(buf), "WiFi:%s MQTT:%s%s",
           m.link == LinkState::WIFI_DOWN ? "OFF" : "OK",
           m.link == LinkState::ONLINE ? "OK" : "OFF",
           m.fault != FaultSim::NONE ? " SIM" : "");
  line(52, buf);
}

static void drawSettings(const UiModel& m) {
  char buf[32];
  oled.setTextSize(1);
  line(0, "CONFIGURACAO");
  snprintf(buf, sizeof(buf), "Setpoint: %.1fC", m.cfg.setpointC);   line(11, buf);
  snprintf(buf, sizeof(buf), "Histerese: %.1fC", m.cfg.hysteresisC); line(21, buf);
  snprintf(buf, sizeof(buf), "Modo: %s", toString(m.cfg.mode));       line(31, buf);
  snprintf(buf, sizeof(buf), "Alerta>%.0f Crit>%.0f", m.cfg.warnC, m.cfg.critC); line(41, buf);
  if (m.secondsSincePublish >= 0) snprintf(buf, sizeof(buf), "Envio:%lds Rst:%s", (long)m.secondsSincePublish, m.resetReason);
  else snprintf(buf, sizeof(buf), "Envio:-- Rst:%s", m.resetReason);
  line(52, buf);
}

void displayRender(const UiModel& m) {
  if (!ready) return;
  oled.clearDisplay();
  if (page == 0) drawMain(m); else drawSettings(m);
  oled.display();
}
