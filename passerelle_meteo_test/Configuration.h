#ifndef CONFIGURATION_PASSERELLE_METEO_H
#define CONFIGURATION_PASSERELLE_METEO_H

#include <Arduino.h>

// Renseigner ces deux valeurs avant un futur televersement.
inline constexpr char WIFI_SSID[] = "A_REMPLIR";
inline constexpr char WIFI_MOT_DE_PASSE[] = "A_REMPLIR";

// Configuration radio commune avec la balise mobile.
inline constexpr float LORA_FREQUENCE_MHZ = 868.0;
inline constexpr float LORA_BANDE_PASSANTE_KHZ = 125.0;
inline constexpr uint8_t LORA_FACTEUR_ETALEMENT = 9;
inline constexpr uint8_t LORA_TAUX_CODAGE = 7;
inline constexpr uint8_t LORA_SYNC_WORD = 0x12;
inline constexpr int8_t LORA_PUISSANCE_DBM = 17;
inline constexpr uint16_t LORA_PREAMBULE = 8;

// Broches de la T-Beam ESP32 classique avec radio SX1276.
inline constexpr int PIN_LORA_SCK = 5;
inline constexpr int PIN_LORA_MISO = 19;
inline constexpr int PIN_LORA_MOSI = 27;
inline constexpr int PIN_LORA_CS = 18;
inline constexpr int PIN_LORA_DIO0 = 26;
inline constexpr int PIN_LORA_RESET = 23;
inline constexpr int PIN_LORA_DIO1 = 33;

inline constexpr int PIN_I2C_SDA = 21;
inline constexpr int PIN_I2C_SCL = 22;

inline constexpr uint32_t DELAI_RECONNEXION_WIFI_MS = 10000;
inline constexpr uint32_t DELAI_HTTP_MS = 8000;
inline constexpr uint32_t DELAI_RECEPTION_LORA_MS = 1200;

#endif
