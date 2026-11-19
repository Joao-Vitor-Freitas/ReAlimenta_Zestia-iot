#pragma once
#include "types.h"
// Botões com interrupção (FALLING) + debounce por tempo dentro da ISR.
void buttonsBegin();
bool buttonsPoll(ButtonEvent& ev);  // não bloqueia
