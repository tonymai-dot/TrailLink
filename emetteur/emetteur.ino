#include "Affichage.h"
#include "Gps.h"
#include "LoraLink.h"
#include <XPowersLib.h> // <-- AJOUTER CETTE LIGNE ICI AUSSI

DonneesAffichage mesInfos;
unsigned long dernierAffichage = 0;
unsigned long dernierEnvoiRadio = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("--- TrailLink EMETTEUR DE SECOURS ---");

    initialiserAffichage();

    // ALLUMAGE DE LA RADIO VIA LE PMU (Correction du type)
    extern XPowersAXP202 PMU; // <-- Changé ici
    PMU.+setBLDO2Voltage(3300); 
    PMU.enableBLDO2();
    delay(100);

    initialiserGps(); 
    initialiserLora(); 

    mesInfos.etatLappareil = AFF_ETAT_SOS; 
}