#include "Affichage.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <XPowersLib.h>

namespace {
constexpr uint8_t PIN_I2C_SDA = 21;
constexpr uint8_t PIN_I2C_SCL = 22;
constexpr uint8_t LARGEUR_ECRAN = 128;
constexpr uint8_t HAUTEUR_ECRAN = 64;
constexpr uint8_t ADRESSE_OLED = 0x3C;
}

XPowersLibInterface *PMU = nullptr;
Adafruit_SSD1306 ecran(LARGEUR_ECRAN, HAUTEUR_ECRAN, &Wire, -1);

static bool initialiserPmu() {
    // T-Beam V1.2 : AXP2101.
    PMU = new XPowersAXP2101(Wire, PIN_I2C_SDA, PIN_I2C_SCL);
    if (PMU->init()) {
        Serial.println(F("[PMU] AXP2101 detecte."));

        // Affectation officielle T-Beam V1.2 :
        // ALDO2 = radio, ALDO3 = GPS.
        // La radio n'est pas encore utilisee par ce sketch : on la coupe
        // pour economiser la batterie et on alimente uniquement le GPS.
        PMU->disablePowerOutput(XPOWERS_ALDO2);
        PMU->setPowerChannelVoltage(XPOWERS_ALDO3, 3300);
        PMU->enablePowerOutput(XPOWERS_ALDO3);
        PMU->enableBattDetection();
        PMU->enableBattVoltageMeasure();
        return true;
    }

    delete PMU;

    // T-Beam V1.1 : AXP192.
    PMU = new XPowersAXP192(Wire, PIN_I2C_SDA, PIN_I2C_SCL);
    if (PMU->init()) {
        Serial.println(F("[PMU] AXP192 detecte."));

        // Affectation officielle T-Beam V1.1 :
        // DCDC1 = ESP32/OLED, LDO2 = radio, LDO3 = GPS.
        PMU->setPowerChannelVoltage(XPOWERS_DCDC1, 3300);
        PMU->enablePowerOutput(XPOWERS_DCDC1);
        PMU->disablePowerOutput(XPOWERS_LDO2);
        PMU->setPowerChannelVoltage(XPOWERS_LDO3, 3300);
        PMU->enablePowerOutput(XPOWERS_LDO3);
        PMU->enableBattDetection();
        PMU->enableBattVoltageMeasure();
        return true;
    }

    delete PMU;
    PMU = nullptr;
    Serial.println(F("[ERREUR] Aucun PMU AXP192/AXP2101 detecte."));
    return false;
}

bool initialiserAffichage() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    if (!initialiserPmu()) {
        Serial.println(F("[AVERTISSEMENT] Essai de l'OLED sans configuration PMU."));
    }

    delay(200);

    if (!ecran.begin(SSD1306_SWITCHCAPVCC, ADRESSE_OLED)) {
        Serial.println(F("[ERREUR] Ecran OLED introuvable a l'adresse 0x3C."));
        return false;
    }

    ecran.clearDisplay();
    ecran.setTextColor(SSD1306_WHITE);
    ecran.setTextSize(1);
    ecran.setCursor(18, 20);
    ecran.println("TrailLink demarre");
    ecran.setCursor(22, 36);
    ecran.println("Recherche GPS...");
    ecran.display();
    Serial.println(F("[OLED] Ecran initialise avec succes."));
    return true;
}

int obtenirPourcentageBatterie() {
    if (PMU == nullptr || !PMU->isBatteryConnect()) {
        return -1;
    }

    const int pourcentage = PMU->getBatteryPercent();
    return (pourcentage >= 0 && pourcentage <= 100) ? pourcentage : -1;
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
    ecran.print(infos.gpsValide ? F("GPS: OK") : F("GPS: NO"));
    ecran.drawLine(0, 11, 128, 11, SSD1306_WHITE);

    ecran.setCursor(0, 18);
    ecran.print("Satellites : ");
    ecran.print(infos.nbSatellites);
    ecran.setCursor(0, 34);
    ecran.print(F("Batterie   : "));
    if (infos.percentBatterie >= 0) {
        ecran.print(infos.percentBatterie);
        ecran.print('%');
    } else {
        ecran.print(F("--"));
    }
    ecran.setCursor(0, 52);
    ecran.print("Statut     : Securise");
    ecran.display();
}

void mettreEcranEnVeille() {
    ecran.clearDisplay();
    ecran.display();
}
