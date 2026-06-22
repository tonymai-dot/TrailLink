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

XPowersPMU PMU;
Adafruit_SSD1306 ecran(LARGEUR_ECRAN, HAUTEUR_ECRAN, &Wire, -1);

bool initialiserAffichage() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, PIN_I2C_SDA, PIN_I2C_SCL)) {
        Serial.println("AXP2101 introuvable sur le bus I2C !");
        return false;
    }

    PMU.setALDO2Voltage(3300); // Alimentation OLED/GPS
    PMU.enableALDO2();
    PMU.setBLDO1Voltage(3300); 
    PMU.enableBLDO1();

    PMU.enableBattDetection();
    PMU.enableVbusVoltageMeasure();
    PMU.enableBattVoltageMeasure();

    if (!ecran.begin(SSD1306_SWITCHCAPVCC, ADRESSE_OLED)) {
        return false;
    }

    ecran.clearDisplay();
    ecran.display();
    return true;
}

// CETTE FONCTION MANQUAIT AU LINKER DE L'ÉMETTEUR :
int obtenirPourcentageBatterie() {
    if (!PMU.isBatteryConnect()) {
        return 100; 
    }
    int pct = PMU.getBatteryPercent();
    if (pct > 100 || pct < 0) {
        return 50; 
    }
    return pct;
}

void rafraichirEcran(const DonneesAffichage& infos) {
    ecran.clearDisplay();
    ecran.setTextColor(SSD1306_WHITE);

    if (infos.etatLappareil == AFF_ETAT_SOS) {
        ecran.setTextSize(3); ecran.setCursor(35, 10); ecran.print("SOS");
        ecran.setTextSize(1); ecran.setCursor(10, 45); ecran.print("ALERTE EN COURS...");
        ecran.display(); return; 
    }
    if (infos.etatLappareil == AFF_ETAT_PRE_ALERTE) {
        ecran.setTextSize(2); ecran.setCursor(10, 2); ecran.print("ATTENTION");
        ecran.setTextSize(1); ecran.setCursor(5, 30); ecran.print("Chute detectee !");
        ecran.display(); return;
    }

    ecran.setTextSize(1);
    ecran.setCursor(0, 0); ecran.print(infos.heure); 
    ecran.setCursor(80, 0); ecran.print(infos.estConnecteReseau ? "LORA: OK" : "LORA: NO");
    ecran.drawLine(0, 11, 128, 11, SSD1306_WHITE);

    ecran.setCursor(0, 18);
    ecran.print("Satellites : "); ecran.print(infos.nbSatellites);
    
    ecran.setCursor(0, 34);
    ecran.print("Batterie   : "); 
    ecran.print((int)infos.percentBatterie); 
    ecran.print("%");
    
    if (PMU.isVbusIn() && PMU.isCharging()) {
        ecran.print(" [CHG]");
    }

    ecran.setCursor(0, 52);
    ecran.print("Statut     : Securise");
    ecran.display();
}

void mettreEcranEnVeille() {
    ecran.clearDisplay();
    ecran.display();
}
