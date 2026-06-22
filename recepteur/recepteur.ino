#include "Affichage.h"
#include "LoraLink.h"
#include <XPowersLib.h> // <-- AJOUTER CETTE LIGNE ICI POUR CORRIGER L'ERREUR 'XPowersPMU'

DonneesAffichage mesInfos;
MessageSecours messageRecu; 
unsigned long dernierAffichage = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("--- TrailLink STATION DE RECEPTION ---");

    initialiserAffichage();

    // ALLUMAGE DE LA RADIO VIA LE PMU (Correction du type)
    extern XPowersAXP202 PMU; // <-- Changé ici
    PMU.setBLDO2Voltage(3300); 
    PMU.enableBLDO2();
    delay(100);

    initialiserLora(); 

    mesInfos.heure = "BASE RX";
    mesInfos.estConnecteReseau = true;
    mesInfos.nbSatellites = 0;
    mesInfos.percentBatterie = 100;
    mesInfos.etatLappareil = AFF_ETAT_NORMAL;
}