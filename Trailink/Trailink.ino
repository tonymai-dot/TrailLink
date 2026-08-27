    #include "Affichage.h"
#include "Gps.h"
#include <sys/time.h>
/*Mai Tony*/

DonneesAffichage mesInfos;
unsigned long dernierAffichage = 0;
bool affichageDisponible = false;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println(F("=== Demarrage TrailLink ==="));

    // Initialisation des deux modules de la T-Beam
    affichageDisponible = initialiserAffichage();
    initialiserGps();

    // Configuration par défaut de l'appareil
    mesInfos.percentBatterie = obtenirPourcentageBatterie();
    mesInfos.gpsValide = false;
    strncpy(mesInfos.heure, "CH.SAT", sizeof(mesInfos.heure));
    mesInfos.heure[sizeof(mesInfos.heure) - 1] = '\0';
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

        Serial.print(F("[GPS] Caracteres recus: "));
        Serial.print(infosGps.caracteresRecus);
        Serial.print(F(" | Satellites: "));
        Serial.print(infosGps.nbSatellites);
        Serial.print(F(" | Position valide: "));
        Serial.println(infosGps.aSignalValide ? F("oui") : F("non"));

        // On injecte les données du GPS dans la structure de l'écran
        mesInfos.nbSatellites = infosGps.nbSatellites;
        mesInfos.percentBatterie = obtenirPourcentageBatterie();
        mesInfos.gpsValide = infosGps.aSignalValide;

        if (infosGps.aSignalValide) {
            // Si le GPS est synchronisé, on affiche la vraie heure reçue du ciel
            char bufferHeure[6];
            sprintf(bufferHeure, "%02d:%02d", infosGps.heure, infosGps.minute);
            strncpy(mesInfos.heure, bufferHeure, sizeof(mesInfos.heure));
            mesInfos.heure[sizeof(mesInfos.heure) - 1] = '\0';
        } else {
            // Tant que le GPS cherche le signal (en intérieur par exemple)
            strncpy(mesInfos.heure, "CH.SAT", sizeof(mesInfos.heure));
            mesInfos.heure[sizeof(mesInfos.heure) - 1] = '\0';
        }

        // Rafraîchissement physique de l'OLED
        if (affichageDisponible) {
            rafraichirEcran(mesInfos);
        }
    }
}
