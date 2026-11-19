#pragma once
#include "types.h"
void actuatorBegin();
// Lógica de borda: classifica a temperatura e decide o cooler (histerese + falha segura).
ActuatorStatus actuatorEvaluate(const Sample& s, const Settings& cfg);
// Reaplica a decisão quando só os parâmetros mudaram (comando remoto, botão).
ActuatorStatus actuatorReapply(const Settings& cfg);
// Pisca o LED crítico e o buzzer sem bloquear.
void actuatorTick(uint32_t nowMs);
