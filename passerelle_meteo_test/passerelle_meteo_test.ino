#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "AlimentationEtRadio.h"
#include "Configuration.h"
#include "ProtocoleMeteo.h"

bool radioDisponible = false;
unsigned long derniereTentativeWifi = 0;

void connecterWifiSiNecessaire();
void traiterDemande(const DemandeMeteo &demande);
void envoyerAccuseReception(const DemandeMeteo &demande);
void envoyerErreur(const DemandeMeteo &demande, StatutMeteo statut);
bool obtenirMeteo(double latitude, double longitude, ReponseMeteo &reponse);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println(F("=== Passerelle meteo TrailLink ==="));

    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);
    connecterWifiSiNecessaire();

    const bool alimentationDisponible = initialiserAlimentation();
    if (!alimentationDisponible) {
        Serial.println(F("[Attention] La radio risque de ne pas etre alimentee."));
    }

    radioDisponible = initialiserRadio();
    if (!radioDisponible) {
        Serial.println(F("[Passerelle] Radio indisponible, redemarrez apres verification."));
    }
}

void loop() {
    connecterWifiSiNecessaire();

    if (!radioDisponible) {
        delay(100);
        return;
    }

    DemandeMeteo demande{};
    int16_t etat = radio.receive(
        reinterpret_cast<uint8_t *>(&demande),
        sizeof(demande),
        DELAI_RECEPTION_LORA_MS
    );

    if (etat == RADIOLIB_ERR_RX_TIMEOUT) {
        return;
    }

    if (etat != RADIOLIB_ERR_NONE) {
        Serial.print(F("[LoRa] Erreur de reception : "));
        Serial.println(etat);
        return;
    }

    if (!enteteValide(demande.entete, TypeMessage::DemandeMeteo)) {
        Serial.println(F("[LoRa] Message ignore : protocole ou type incorrect."));
        return;
    }

    traiterDemande(demande);
}

void connecterWifiSiNecessaire() {
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    const unsigned long maintenant = millis();
    if (derniereTentativeWifi != 0 &&
        maintenant - derniereTentativeWifi < DELAI_RECONNEXION_WIFI_MS) {
        return;
    }

    derniereTentativeWifi = maintenant;
    Serial.print(F("[Wi-Fi] Connexion a "));
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_MOT_DE_PASSE);
}

void traiterDemande(const DemandeMeteo &demande) {
    Serial.print(F("[LoRa] Demande meteo recue de la balise "));
    Serial.println(demande.entete.idBalise);

    envoyerAccuseReception(demande);

    if (!coordonneesValides(demande)) {
        envoyerErreur(demande, StatutMeteo::CoordonneesInvalides);
        return;
    }

    if (WiFi.status() != WL_CONNECTED) {
        envoyerErreur(demande, StatutMeteo::WifiIndisponible);
        return;
    }

    ReponseMeteo reponse{};
    reponse.entete = creerEntete(
        TypeMessage::ReponseMeteo,
        demande.entete.idBalise,
        demande.entete.idDemande
    );

    const double latitude = demande.latitudeE7 / 10000000.0;
    const double longitude = demande.longitudeE7 / 10000000.0;

    if (!obtenirMeteo(latitude, longitude, reponse)) {
        reponse.statut = StatutMeteo::ServiceIndisponible;
    }

    int16_t etat = radio.transmit(
        reinterpret_cast<uint8_t *>(&reponse),
        sizeof(reponse)
    );

    Serial.print(F("[LoRa] Reponse meteo : "));
    Serial.println(etat == RADIOLIB_ERR_NONE ? F("envoyee") : F("echec"));
}

void envoyerAccuseReception(const DemandeMeteo &demande) {
    AccuseReception accuse{};
    accuse.entete = creerEntete(
        TypeMessage::AccuseReception,
        demande.entete.idBalise,
        demande.entete.idDemande
    );

    radio.transmit(reinterpret_cast<uint8_t *>(&accuse), sizeof(accuse));
}

void envoyerErreur(const DemandeMeteo &demande, StatutMeteo statut) {
    ReponseMeteo reponse{};
    reponse.entete = creerEntete(
        TypeMessage::ReponseMeteo,
        demande.entete.idBalise,
        demande.entete.idDemande
    );
    reponse.statut = statut;

    radio.transmit(reinterpret_cast<uint8_t *>(&reponse), sizeof(reponse));
}

bool obtenirMeteo(double latitude, double longitude, ReponseMeteo &reponse) {
    String url = F("https://api.open-meteo.com/v1/forecast?latitude=");
    url += String(latitude, 6);
    url += F("&longitude=");
    url += String(longitude, 6);
    url += F("&current=temperature_2m,wind_speed_10m,wind_gusts_10m&wind_speed_unit=kmh");

    WiFiClientSecure client;
    // Suffisant pour le prototype. Une version finale devra verifier le certificat TLS.
    client.setInsecure();

    HTTPClient http;
    http.setConnectTimeout(DELAI_HTTP_MS);
    http.setTimeout(DELAI_HTTP_MS);

    if (!http.begin(client, url)) {
        Serial.println(F("[Meteo] Impossible de preparer la requete HTTP."));
        return false;
    }

    const int codeHttp = http.GET();
    if (codeHttp != HTTP_CODE_OK) {
        Serial.print(F("[Meteo] Erreur HTTP : "));
        Serial.println(codeHttp);
        http.end();
        return false;
    }

    JsonDocument document;
    DeserializationError erreurJson = deserializeJson(document, http.getStream());
    http.end();

    if (erreurJson || document["current"].isNull()) {
        Serial.print(F("[Meteo] Reponse JSON invalide : "));
        Serial.println(erreurJson.c_str());
        return false;
    }

    JsonObject meteo = document["current"];
    if (meteo["temperature_2m"].isNull() ||
        meteo["wind_speed_10m"].isNull() ||
        meteo["wind_gusts_10m"].isNull()) {
        Serial.println(F("[Meteo] Une valeur demandee est absente."));
        return false;
    }

    const float temperature = meteo["temperature_2m"].as<float>();
    const float vent = meteo["wind_speed_10m"].as<float>();
    const float rafales = meteo["wind_gusts_10m"].as<float>();

    reponse.statut = StatutMeteo::Ok;
    reponse.temperatureDixiemeC = static_cast<int16_t>(lroundf(temperature * 10.0f));
    reponse.ventDixiemeKmh = static_cast<uint16_t>(max(0L, lroundf(vent * 10.0f)));
    reponse.rafalesDixiemeKmh = static_cast<uint16_t>(max(0L, lroundf(rafales * 10.0f)));

    Serial.print(F("[Meteo] Temperature: "));
    Serial.print(temperature, 1);
    Serial.print(F(" C | Vent: "));
    Serial.print(vent, 1);
    Serial.print(F(" km/h | Rafales: "));
    Serial.print(rafales, 1);
    Serial.println(F(" km/h"));
    return true;
}
