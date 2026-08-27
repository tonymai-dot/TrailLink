#ifndef ALIMENTATION_ET_RADIO_H
#define ALIMENTATION_ET_RADIO_H

#include <Arduino.h>
#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include <XPowersLib.h>

#include "Configuration.h"

inline XPowersAXP192 alimentation(Wire, PIN_I2C_SDA, PIN_I2C_SCL);
inline SX1276 radio = new Module(
    PIN_LORA_CS,
    PIN_LORA_DIO0,
    PIN_LORA_RESET,
    PIN_LORA_DIO1
);

inline bool initialiserAlimentation() {
    Serial.print(F("[Alimentation] Recherche de l'AXP192... "));

    if (!alimentation.init()) {
        Serial.println(F("introuvable"));
        return false;
    }

    // Sur la T-Beam classique, LDO2 alimente la radio LoRa.
    alimentation.setLDO2Voltage(3300);
    alimentation.enableLDO2();
    Serial.println(F("OK"));
    delay(100);
    return true;
}

inline bool initialiserRadio() {
    Serial.print(F("[LoRa] Initialisation du SX1276... "));

    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, PIN_LORA_CS);

    int16_t etat = radio.begin(
        LORA_FREQUENCE_MHZ,
        LORA_BANDE_PASSANTE_KHZ,
        LORA_FACTEUR_ETALEMENT,
        LORA_TAUX_CODAGE,
        LORA_SYNC_WORD,
        LORA_PUISSANCE_DBM,
        LORA_PREAMBULE
    );

    if (etat != RADIOLIB_ERR_NONE) {
        Serial.print(F("echec, code "));
        Serial.println(etat);
        return false;
    }

    Serial.println(F("OK"));
    return true;
}

#endif
