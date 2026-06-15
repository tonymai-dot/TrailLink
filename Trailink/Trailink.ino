    #include "Affichage.h"
#include "Gps.h"
#include <sys/time.h>
/*Mai Tony*/

DonneesAffichage mesInfos;
unsigned long dernierAffichage = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialisation des deux modules de la T-Beam
    initialiserAffichage();
    initialiserGps();

    // Configuration par défaut de l'appareil
    mesInfos.percentBatterie = 88.0; 
    mesInfos.estConnecteReseau = true;
    mesInfos.etatLappareil = AFF_ETAT_NORMAL;
}

void loop() {
    // 1. Écoute permanente et obligatoire du flux GPS (ne pas bloquer avec des delay ici !)
    mettreAJourGps();

    // 2. On met à jour l'écran toutes les secondes (sans bloquer le GPS)
    if (millis() - dernierAffichage >= 1000) {
        dernierAffichage = millis();

        // On récupère les données réelles du GPS
        DonneesGps infosGps = obtenirDonneesGps();

        // On injecte les données du GPS dans la structure de l'écran
        mesInfos.nbSatellites = infosGps.nbSatellites;

        if (infosGps.aSignalValide) {
            // Si le GPS est synchronisé, on affiche la vraie heure reçue du ciel
            char bufferHeure[6];
            sprintf(bufferHeure, "%02d:%02d", infosGps.heure, infosGps.minute);
            mesInfos.heure = String(bufferHeure);
            mesInfos.estConnecteReseau = true;
        } else {
            // Tant que le GPS cherche le signal (en intérieur par exemple)
            mesInfos.heure = "CH. SAT";
            mesInfos.estConnecteReseau = false;
        }

        // Rafraîchissement physique de l'OLED
        rafraichirEcran(mesInfos);
    }
}