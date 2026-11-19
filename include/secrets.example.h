#pragma once
// Copie este arquivo para include/secrets.h e preencha. NÃO versione o secrets.h.
// No Wokwi a rede WiFi é "Wokwi-GUEST" (sem senha).
#define WIFI_SSID        "Wokwi-GUEST"
#define WIFI_PASS        ""
#define WIFI_CHANNEL     6

// ThingSpeak > Devices > MQTT > Add a new device (autorize os 2 canais abaixo)
#define MQTT_HOST        "mqtt3.thingspeak.com"
#define MQTT_PORT        1883
#define MQTT_CLIENT_ID   "COLE_O_CLIENT_ID"
#define MQTT_USER        "COLE_O_USERNAME"
#define MQTT_PASS        "COLE_A_SENHA"

// IDs numéricos dos canais (aparecem na página de cada canal)
#define TS_CH_TELEMETRY  "0000000"   // dispositivo -> nuvem (fields 1 a 8)
#define TS_CH_COMMANDS   "0000000"   // dashboard -> dispositivo (field1 = setpoint, field2 = modo)
