#include "sensor.h"
#include <DHT.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_timer.h>
#include "config.h"
#include "watchdog.h"

static DHT dht(PIN_DHT, DHT22);
static QueueHandle_t sampleQueue = nullptr;

// Tarefa de amostragem: acorda em instantes fixos (múltiplos de SAMPLE_PERIOD_MS),
// independente do tempo gasto na leitura. Não usa delay() no loop principal.
static void sensorTask(void*) {
  wdtSubscribeCurrentTask();
  dht.begin();
  TickType_t wakeTime = xTaskGetTickCount();
  int64_t prevUs = 0;
  for (;;) {
    vTaskDelayUntil(&wakeTime, pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
    int64_t nowUs = esp_timer_get_time();

    Sample s;
    s.timestampMs = (uint32_t)(nowUs / 1000);
    s.periodUs = prevUs ? (uint32_t)(nowUs - prevUs) : 0;
    prevUs = nowUs;
    float t = dht.readTemperature(false, true);  // force = lê o sensor de fato
    float h = dht.readHumidity();
    s.valid = !isnan(t) && !isnan(h);
    s.tempC = s.valid ? t : NAN;
    s.humidity = s.valid ? h : NAN;

    xQueueSend(sampleQueue, &s, 0);  // não bloqueia: o loop() esvazia a fila a cada volta
    wdtFeed();
  }
}

bool sensorBegin() {
  sampleQueue = xQueueCreate(8, sizeof(Sample));
  if (!sampleQueue) return false;
  // Núcleo 1, prioridade acima do loop(): garante a pontualidade da amostragem
  return xTaskCreatePinnedToCore(sensorTask, "sensor", 4096, nullptr, 2, nullptr, 1) == pdPASS;
}

bool sensorReceive(Sample& out) {
  return sampleQueue && xQueueReceive(sampleQueue, &out, 0) == pdTRUE;
}
