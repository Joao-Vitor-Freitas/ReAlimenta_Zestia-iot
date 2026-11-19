#include "watchdog.h"
#include <esp_task_wdt.h>
#include <esp_idf_version.h>
#include "config.h"

void wdtBegin() {
#if ESP_IDF_VERSION_MAJOR >= 5
  esp_task_wdt_config_t c;
  c.timeout_ms = WDT_TIMEOUT_S * 1000;
  c.idle_core_mask = 0;
  c.trigger_panic = true;
  if (esp_task_wdt_init(&c) == ESP_ERR_INVALID_STATE) esp_task_wdt_reconfigure(&c);
#else
  esp_task_wdt_init(WDT_TIMEOUT_S, true);  // true = reinicia o chip ao estourar
#endif
  esp_task_wdt_add(NULL);  // tarefa loop()
}
void wdtSubscribeCurrentTask() { esp_task_wdt_add(NULL); }
void wdtFeed() { esp_task_wdt_reset(); }
