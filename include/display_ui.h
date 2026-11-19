#pragma once
#include "types.h"
bool displayBegin();
void displayTogglePage();
void displayRender(const UiModel& m);  // página 0: medição e status; página 1: configuração
