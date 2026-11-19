#pragma once
#include "types.h"
using CommandHandler = void (*)(const RemoteCommand&);

void connectivityBegin(CommandHandler handler);
void connectivityLoop(uint32_t nowMs);          // máquina de estados não bloqueante (WiFi -> MQTT -> online)
LinkState connectivityState();
bool connectivityPublish(const Telemetry& t);   // ThingSpeak: channels/<id>/publish
void connectivityCycleFault();                  // NONE -> MQTT_BLOCKED -> WIFI_BLOCKED -> NONE
FaultSim connectivityFault();
int connectivityRssi();
