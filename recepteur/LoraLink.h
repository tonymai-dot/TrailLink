// LoraLink.h
#ifndef LORALINK_H
#define LORALINK_H

#include <Arduino.h>
#include <RadioLib.h>

// Définition des broches LoRa physiques de la T-Beam (valable pour SX1262 / SX1276)
#define LORA_PIN_NSS  18
#define LORA_PIN_DIO1 4  // Note: Utilise 26 ou 23 si tu as une très vieille T-Beam V1.0
#define LORA_PIN_NRST 23
#define LORA_PIN_BUSY 32

// Configuration de la puce radio LoRa (SX1262 courante sur les T-Beam AXP2101)
inline SX1262 radio = new Module(LORA_PIN_NSS, LORA_PIN_DIO1, LORA_PIN_NRST, LORA_PIN_BUSY);

// Structure propre du message à envoyer dans les airs
struct MessageSecours {
    float latitude;
    float longitude;
    int batterie;
    int etat;
};

/**
 * @brief Initialise le module radio LoRa à la fréquence européenne.
 */
inline bool initialiserLora() {
    Serial.print(F("[LoRa] Initialisation ... "));
    
    // Démarrage à 868.0 MHz, bande passante 125 kHz, codage 4/7, puissance 22 dBm
    int state = radio.begin(868.0, 125.0, 9, 7, 0x12, 22, 8);
    
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("OK !"));
        return true;
    } else {
        Serial.print(F("ECHEC, code erreur : "));
        Serial.println(state);
        return false;
    }
}

/**
 * @brief Envoie les coordonnées GPS brutes par radio.
 */
inline void envoyerPosition(float lat, float lng, int bat, int etat) {
    MessageSecours msg = {lat, lng, bat, etat};
    
    Serial.print(F("[LoRa] Envoi du SOS ... "));
    
    // On envoie la structure directement convertie en octets dans les airs
    int state = radio.transmit((uint8_t*)&msg, sizeof(MessageSecours));

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("Envoye avec succes !"));
    } else {
        Serial.print(F("Erreur de transmission : "));
        Serial.println(state);
    }
}

/**
 * @brief Écoute l'espace radio pour voir si une autre carte appelle à l'aide.
 * @param msgSortant Référence vers la structure à remplir si un message arrive.
 * @return true si un message valide a été capté, false sinon.
 */
inline bool ecouterRadio(MessageSecours &msgSortant) {
    // On regarde si un signal est arrivé dans la puce
    size_t length = sizeof(MessageSecours);
    int state = radio.receive((uint8_t*)&msgSortant, length);

    if (state == RADIOLIB_ERR_NONE) {
        return true; // Un message valide est copié dans msgSortant
    }
    return false;
}

#endif // LORALINK_H