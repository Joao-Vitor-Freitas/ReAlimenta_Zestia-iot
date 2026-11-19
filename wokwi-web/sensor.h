#pragma once
#include "types.h"
// Cria a tarefa de amostragem periódica (vTaskDelayUntil => período sem deriva).
bool sensorBegin();
// Retira uma amostra da fila sem bloquear. Retorna false se não houver nenhuma.
bool sensorReceive(Sample& out);
