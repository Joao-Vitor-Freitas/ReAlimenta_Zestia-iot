#include "buttons.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "config.h"

struct ButtonCfg {
  uint8_t pin;
  ButtonEvent ev;
  volatile uint32_t lastMs;
};
static ButtonCfg buttons[] = {
  { PIN_BTN_MODE, ButtonEvent::MODE, 0 },
  { PIN_BTN_PAGE, ButtonEvent::PAGE, 0 },
  { PIN_BTN_FAULT, ButtonEvent::FAULT, 0 },
};
static QueueHandle_t evQueue = nullptr;

// ISR: descarta bordas que chegam antes de DEBOUNCE_MS do último evento aceito
static void IRAM_ATTR onButton(void* arg) {
  ButtonCfg* b = static_cast<ButtonCfg*>(arg);
  uint32_t now = millis();
  if (now - b->lastMs < DEBOUNCE_MS) return;
  b->lastMs = now;
  BaseType_t woken = pdFALSE;
  xQueueSendFromISR(evQueue, &b->ev, &woken);
  if (woken) portYIELD_FROM_ISR();
}

void buttonsBegin() {
  evQueue = xQueueCreate(8, sizeof(ButtonEvent));
  for (ButtonCfg& b : buttons) {
    pinMode(b.pin, INPUT_PULLUP);
    attachInterruptArg(b.pin, onButton, &b, FALLING);
  }
}

bool buttonsPoll(ButtonEvent& ev) {
  return evQueue && xQueueReceive(evQueue, &ev, 0) == pdTRUE;
}
