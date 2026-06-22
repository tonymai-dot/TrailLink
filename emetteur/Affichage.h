#ifndef AFFICHAGE_H
#define AFFICHAGE_H

#include <Arduino.h>

const int AFF_ETAT_NORMAL     = 0;
const int AFF_ETAT_PRE_ALERTE = 1;
const int AFF_ETAT_SOS        = 2;

struct DonneesAffichage {
    float percentBatterie;    
    int nbSatellites;         
    String heure;             
    bool estConnecteReseau;   
    int etatLappareil;        
};

bool initialiserAffichage();
void rafraichirEcran(const DonneesAffichage& infos);
void mettreEcranEnVeille();

// --- LA LIGNE À RAJOUTER ICI : ---
int obtenirPourcentageBatterie(); 

#endif // AFFICHAGE_H