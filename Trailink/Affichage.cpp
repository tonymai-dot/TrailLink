#include "Affichage.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define XPOWERS_CHIP_AXP2101
#include <XPowersLib.h>

#define PIN_I2C_SDA     21
#define PIN_I2C_SCL     22
#define LARGEUR_ECRAN   128
#define HAUTEUR_ECRAN   64
#define ADRESSE_OLED    0x3C
#include <TinyGPS++.h>

TinyGPSPlus gps;
// Supposons que ton flux GPS arrive sur Serial1 (broches de la T-Beam)

String obtenirHeureExacte() {
    if (gps.time.isValid()) {
        int heureLocale = gps.time.hour() + 2; // +2 pour l'heure d'été en France (2026)
        if (heureLocale >= 24) heureLocale -= 24;

        String h = (heureLocale < 10) ? "0" + String(heureLocale) : String(heureLocale);
        String m = (gps.time.minute() < 10) ? "0" + String(gps.time.minute()) : String(gps.time.minute());
        
        return h + ":" + m; // Renvoie par exemple "14:35"
    }
    return "--:--"; // Si le GPS n'a pas encore de signal
}
XPowersPMU PMU;
Adafruit_SSD1306 ecran(LARGEUR_ECRAN, HAUTEUR_ECRAN, &Wire, -1);

bool initialiserAffichage() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, PIN_I2C_SDA, PIN_I2C_SCL)) {
        return false;
    }

    PMU.setALDO2Voltage(3300); 
    PMU.enableALDO2();
    PMU.setBLDO1Voltage(3300); 
    PMU.enableBLDO1();

    if (!ecran.begin(SSD1306_SWITCHCAPVCC, ADRESSE_OLED)) {
        return false;
    }

    ecran.clearDisplay();
    ecran.display();
    return true;
}

void rafraichirEcran(const DonneesAffichage& infos) {
    ecran.clearDisplay();
    ecran.setTextColor(SSD1306_WHITE);

    if (infos.etatLappareil == AFF_ETAT_SOS) {
        ecran.setTextSize(3);
        ecran.setCursor(35, 10);
        ecran.print("SOS");
        ecran.setTextSize(1);
        ecran.setCursor(10, 45);
        ecran.print("ALERTE EN COURS...");
        ecran.display();
        return; 
    }

    if (infos.etatLappareil == AFF_ETAT_PRE_ALERTE) {
        ecran.setTextSize(2);
        ecran.setCursor(10, 2);
        ecran.print("ATTENTION");
        ecran.setTextSize(1);
        ecran.setCursor(5, 30);
        ecran.print("Chute detectee !");
        ecran.display();
        return;
    }

    ecran.setTextSize(1);
    ecran.setCursor(0, 0);
    ecran.print(infos.heure); 
    ecran.setCursor(80, 0);
    ecran.print(infos.estConnecteReseau ? "LORA: OK" : "LORA: NO");
    ecran.drawLine(0, 11, 128, 11, SSD1306_WHITE);

    ecran.setCursor(0, 18);
    ecran.print("Satellites : "); ecran.print(infos.nbSatellites);
    ecran.setCursor(0, 34);
    ecran.print("Batterie   : "); ecran.print((int)infos.percentBatterie); ecran.print("%");
    ecran.setCursor(0, 52);
    ecran.print("Statut     : Securise");

    ecran.display();
}

void mettreEcranEnVeille() {
    ecran.clearDisplay();
    ecran.display();
}