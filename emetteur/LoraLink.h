// LoraLink.h (Version corrigée pour T-Beam SX1276)
#ifndef LORALINK_H
#define LORALINK_H

#include <Arduino.h>
#include <RadioLib.h>

// Définition des broches LoRa physiques pour la T-Beam SX1276
#define LORA_PIN_NSS  18
#define LORA_PIN_DIO0 26  // La SX1276 utilise DIO0 pour signaler la fin d'envoi
#define LORA_PIN_NRST 23
#define LORA_PIN_DIO1 33  // Broche secondaire

// ATTENTION : On instancie une "SX1276" ici au lieu d'une SX1262 !
inline SX1276 radio = new Module(LORA_PIN_NSS, LORA_PIN_DIO0, LORA_PIN_NRST, LORA_PIN_DIO1);

// Structure propre du message à envoyer dans les airs
struct MessageSecours {
    float latitude;
    float longitude;
    int batterie;
    int etat;
};

/**
 * @brief Initialise le module radio LoRa SX1276.
 */
inline bool initialiserLora() {
    Serial.print(F("[LoRa] Initialisation du module SX1276... "));
    
    // Initialisation standard à 868.0 MHz (Europe)
    // Paramètres : Fréquence = 868.0, Bande passante = 125.0, Spreading Factor = 9, Coding Rate = 7
    int state = radio.begin(868.0, 125.0, 9, 7);
    
    if (state == RADIOLIB_ERR_NONE) {
        // Optionnel : On augmente la puissance d'émission à 20 dBm (Maximum pour SX1276)
        radio.setOutputPower(20);
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
    
    // Transmission des données
    int state = radio.transmit((uint8_t*)&msg, sizeof(MessageSecours));

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("Envoye avec succes !"));
    } else {
        Serial.print(F("Erreur de transmission : "));
        Serial.println(state);
    }
}

/**
 * @brief Écoute l'espace radio (pour le récepteur).
 */
inline bool ecouterRadio(MessageSecours &msgSortant) {
    size_t length = sizeof(MessageSecours);
    int state = radio.receive((uint8_t*)&msgSortant, length);

    if (state == RADIOLIB_ERR_NONE) {
        return true; 
    }
    return false;
}

#endif // LORALINK_H