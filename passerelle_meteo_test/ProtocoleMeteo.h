#ifndef PROTOCOLE_METEO_H
#define PROTOCOLE_METEO_H

#include <Arduino.h>
#include <math.h>

inline constexpr uint16_t PROTOCOLE_SIGNATURE = 0x544C; // "TL"
inline constexpr uint8_t PROTOCOLE_VERSION = 1;

enum class TypeMessage : uint8_t {
    DemandeMeteo = 1,
    AccuseReception = 2,
    ReponseMeteo = 3
};

enum class StatutMeteo : uint8_t {
    Ok = 0,
    WifiIndisponible = 1,
    ServiceIndisponible = 2,
    ReponseInvalide = 3,
    CoordonneesInvalides = 4
};

#pragma pack(push, 1)
struct EnteteMessage {
    uint16_t signature;
    uint8_t version;
    TypeMessage type;
    uint16_t idBalise;
    uint32_t idDemande;
};

struct DemandeMeteo {
    EnteteMessage entete;
    int32_t latitudeE7;
    int32_t longitudeE7;
};

struct AccuseReception {
    EnteteMessage entete;
};

struct ReponseMeteo {
    EnteteMessage entete;
    StatutMeteo statut;
    int16_t temperatureDixiemeC;
    uint16_t ventDixiemeKmh;
    uint16_t rafalesDixiemeKmh;
};
#pragma pack(pop)

inline EnteteMessage creerEntete(TypeMessage type, uint16_t idBalise, uint32_t idDemande) {
    return {PROTOCOLE_SIGNATURE, PROTOCOLE_VERSION, type, idBalise, idDemande};
}

inline bool enteteValide(const EnteteMessage &entete, TypeMessage typeAttendu) {
    return entete.signature == PROTOCOLE_SIGNATURE &&
           entete.version == PROTOCOLE_VERSION &&
           entete.type == typeAttendu;
}

inline bool coordonneesValides(const DemandeMeteo &demande) {
    const double latitude = demande.latitudeE7 / 10000000.0;
    const double longitude = demande.longitudeE7 / 10000000.0;
    return isfinite(latitude) && isfinite(longitude) &&
           latitude >= -90.0 && latitude <= 90.0 &&
           longitude >= -180.0 && longitude <= 180.0;
}

static_assert(sizeof(DemandeMeteo) == 18, "Taille inattendue de DemandeMeteo");
static_assert(sizeof(ReponseMeteo) == 17, "Taille inattendue de ReponseMeteo");

#endif
