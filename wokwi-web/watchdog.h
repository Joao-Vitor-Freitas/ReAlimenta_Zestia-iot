#pragma once
void wdtBegin();                  // configura o watchdog de tarefas e inscreve a tarefa loop()
void wdtSubscribeCurrentTask();   // inscreve a tarefa que chamar
void wdtFeed();                   // "alimenta" o watchdog
