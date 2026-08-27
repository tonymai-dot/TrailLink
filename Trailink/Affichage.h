#ifndef AFFICHAGE_H
#define AFFICHAGE_H

#include <Arduino.h>

enum EtatAppareil : uint8_t {
    AFF_ETAT_NORMAL = 0,
    AFF_ETAT_PRE_ALERTE,
    AFF_ETAT_SOS
};

struct DonneesAffichage {
    int percentBatterie;
    uint8_t nbSatellites;
    char heure[7];
    bool gpsValide;
    EtatAppareil etatLappareil;
};

bool initialiserAffichage();
void rafraichirEcran(const DonneesAffichage& infos);
void mettreEcranEnVeille();
int obtenirPourcentageBatterie();


#endif // AFFICHAGE_H
