// Gps.h
#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include <TinyGPS++.h>

// --- CONFIGURATION MATÉRIELLE DU GPS (T-BEAM AXP2101) ---
#define PIN_GPS_RX 34
#define PIN_GPS_TX 12
#define GPS_BAUDRATE 9600 // Vitesse standard des modules Neo-6M/M8N

// Structure pour exporter proprement les données lues vers le Main
struct DonneesGps {
    int nbSatellites;
    int heure;
    int minute;
    bool aSignalValide;
};

// On crée les instances globales en "inline" pour éviter les erreurs de doublons
inline TinyGPSPlus gps;
// HardwareSerial 1 de l'ESP32 pour communiquer avec le module GPS
inline HardwareSerial SerialGPS(1); 

/**
 * @brief Initialise la liaison série avec le module GPS de la T-Beam.
 */
inline void initialiserGps() {
    // Initialise le port série du GPS avec les bonnes broches de la T-Beam
    SerialGPS.begin(GPS_BAUDRATE, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    Serial.println("Liaison materielle avec le GPS initialisee.");
}

/**
 * @brief Lit en continu les données brutes envoyées par le GPS et les décode.
 * À placer impérativement au tout début du loop() principal.
 */
inline void mettreAJourGps() {
    while (SerialGPS.available() > 0) {
        gps.encode(SerialGPS.read());
    }
}

/**
 * @brief Extrait et organise les données utiles décodées par TinyGPS++
 * @return Une structure DonneesGps prête à l'emploi.
 */
inline DonneesGps obtenirDonneesGps() {
    DonneesGps donnees;
    
    // 1. Récupération du nombre de satellites
    if (gps.satellites.isValid()) {
        donnees.nbSatellites = gps.satellites.value();
    } else {
        donnees.nbSatellites = 0;
    }

    // 2. Vérification de la validité du signal complet
    donnees.aSignalValide = gps.location.isValid() && gps.time.isValid();

    // 3. Récupération et conversion de l'heure UTC en heure locale (France)
    if (gps.time.isValid()) {
        int h = gps.time.hour();
        int m = gps.time.minute();
        
        // Ajustement Heure d'été France (+2 heures)
        // Note: En 2026, l'heure d'été est active de fin mars à fin octobre.
        h += 2; 
        if (h >= 24) {
            h -= 24;
        }
        
        donnees.heure = h;
        donnees.minute = m;
    } else {
        donnees.heure = 0;
        donnees.minute = 0;
    }

    return donnees;
}

#endif // GPS_H