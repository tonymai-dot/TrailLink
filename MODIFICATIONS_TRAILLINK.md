# TrailLink — Modifications réalisées

## 1. Objectif de ce document

Ce document présente les différences entre :

- le programme TrailLink présent au départ sur la branche `branch` ;
- la version locale corrigée et optimisée pour la carte LilyGO T-Beam.

Pour chaque modification, le document explique :

1. le fonctionnement précédent ;
2. le problème rencontré ;
3. la nouvelle solution ;
4. l'utilité concrète de la modification.

Les fichiers concernés sont :

```text
Trailink/Trailink.ino
Trailink/Affichage.h
Trailink/Affichage.cpp
Trailink/gps.h
```

La documentation générale du programme reste disponible dans :

```text
DOCUMENTATION_TRAILLINK.md
```

---

## 2. Résumé des principales différences

| Avant | Maintenant | Pourquoi |
|---|---|---|
| Seul l'AXP2101 était accepté | Détection automatique AXP192/AXP2101 | Compatibilité avec plusieurs versions de T-Beam |
| Mauvaises sorties d'alimentation | Sorties adaptées au PMU détecté | Alimentation correcte du GPS et de l'écran |
| Radio alimentée inutilement | Radio éteinte | Réduction de la consommation |
| Batterie toujours affichée à 88 % | Véritable mesure du PMU | Affichage réaliste |
| Très peu de diagnostics | Messages PMU, OLED et GPS | Recherche de panne plus simple |
| Écran utilisé même après un échec | Écran utilisé seulement s'il fonctionne | Exécution plus sûre |
| Affichage `LORA: OK/NO` | Affichage `GPS: OK/NO` | Correspond au fonctionnement réel |
| Deux objets GPS différents | Un seul objet GPS | Données cohérentes |
| Heure stockée avec `String` | Tableau de caractères fixe | Mémoire plus stable |
| Données parfois non initialisées | Initialisation systématique | Évite les valeurs inconnues |
| Types `int` utilisés partout | Petits types adaptés | Structures plus légères |
| Textes conservés en RAM | Textes fixes placés en mémoire programme | Davantage de RAM disponible |

---

## 3. Détection du gestionnaire d'alimentation

### Avant

Le programme imposait l'utilisation d'un AXP2101 :

```cpp
#define XPOWERS_CHIP_AXP2101
XPowersPMU PMU;
```

Une carte équipée d'un AXP192 ne pouvait donc pas être correctement initialisée.

En cas d'échec, la fonction retournait simplement `false`, sans expliquer le problème :

```cpp
if (!PMU.begin(...)) {
    return false;
}
```

### Maintenant

Le programme essaie d'abord un AXP2101 :

```cpp
PMU = new XPowersAXP2101(
    Wire,
    PIN_I2C_SDA,
    PIN_I2C_SCL
);
```

Si cet essai échoue, il essaie un AXP192 :

```cpp
PMU = new XPowersAXP192(
    Wire,
    PIN_I2C_SDA,
    PIN_I2C_SCL
);
```

Le modèle détecté apparaît dans le moniteur série :

```text
[PMU] AXP2101 detecte.
```

ou :

```text
[PMU] AXP192 detecte.
```

Si aucun des deux composants ne répond :

```text
[ERREUR] Aucun PMU AXP192/AXP2101 detecte.
```

### Utilité

La version exacte de la carte n'a plus besoin d'être choisie manuellement dans le code.

Le même programme peut démarrer sur une T-Beam utilisant un AXP192 ou un AXP2101.

---

## 4. Correction des sorties d'alimentation

### Avant

Le programme activait :

```cpp
PMU.setALDO2Voltage(3300);
PMU.enableALDO2();

PMU.setBLDO1Voltage(3300);
PMU.enableBLDO1();
```

Sur une T-Beam AXP2101 :

```text
ALDO2 = radio LoRa
ALDO3 = GPS
BLDO1 = inutilisé
```

Le programme alimentait donc la radio et une sortie inutilisée, mais pas la sortie GPS prévue par le constructeur.

### Maintenant avec AXP2101

La bonne sortie GPS est réglée sur 3,3 volts :

```cpp
PMU->setPowerChannelVoltage(
    XPOWERS_ALDO3,
    3300
);

PMU->enablePowerOutput(XPOWERS_ALDO3);
```

### Maintenant avec AXP192

Le programme utilise :

```text
DCDC1 = ESP32 et écran
LDO3  = GPS
```

Le code correspondant est :

```cpp
PMU->setPowerChannelVoltage(
    XPOWERS_DCDC1,
    3300
);
PMU->enablePowerOutput(XPOWERS_DCDC1);

PMU->setPowerChannelVoltage(
    XPOWERS_LDO3,
    3300
);
PMU->enablePowerOutput(XPOWERS_LDO3);
```

### Utilité

Le GPS reçoit maintenant son alimentation depuis la sortie prévue pour la version de T-Beam détectée.

Cette correction peut résoudre un GPS qui reste totalement silencieux parce qu'il n'était pas alimenté.

---

## 5. Radio LoRa désactivée

### Avant

La sortie d'alimentation de la radio pouvait être active alors que `Trailink.ino` n'utilisait pas LoRa.

### Maintenant

Avec un AXP2101 :

```cpp
PMU->disablePowerOutput(XPOWERS_ALDO2);
```

Avec un AXP192 :

```cpp
PMU->disablePowerOutput(XPOWERS_LDO2);
```

### Utilité

La radio ne consomme pas d'énergie inutilement.

Elle reste désactivée tant que sa référence exacte et ses broches ne sont pas confirmées.

Lorsque LoRa sera intégré au programme, cette sortie devra être réactivée avant l'initialisation de RadioLib.

---

## 6. Diagnostic du PMU

### Avant

Un échec d'initialisation du PMU ne produisait pas de message utile.

### Maintenant

Le programme indique clairement :

```text
[PMU] AXP192 detecte.
```

```text
[PMU] AXP2101 detecte.
```

ou :

```text
[ERREUR] Aucun PMU AXP192/AXP2101 detecte.
```

Si le PMU n'est pas détecté, l'écran est quand même testé :

```text
[AVERTISSEMENT] Essai de l'OLED sans configuration PMU.
```

### Utilité

Le moniteur série permet maintenant de différencier :

- une erreur de PMU ;
- une erreur d'écran ;
- une erreur de GPS.

---

## 7. Initialisation de l'écran

### Avant

Si l'écran n'était pas trouvé, la fonction retournait `false` :

```cpp
if (!ecran.begin(...)) {
    return false;
}
```

Le programme principal ne vérifiait pas cette valeur et continuait à envoyer des commandes à l'écran.

### Maintenant

Le résultat est mémorisé :

```cpp
affichageDisponible = initialiserAffichage();
```

L'écran n'est actualisé que s'il est disponible :

```cpp
if (affichageDisponible) {
    rafraichirEcran(mesInfos);
}
```

En cas d'échec :

```text
[ERREUR] Ecran OLED introuvable a l'adresse 0x3C.
```

En cas de réussite :

```text
[OLED] Ecran initialise avec succes.
```

### Utilité

Une panne d'écran n'empêche plus le GPS et le moniteur série de fonctionner.

Le programme évite également d'utiliser un objet écran qui n'a pas été correctement initialisé.

---

## 8. Page de démarrage ajoutée

### Avant

L'écran était effacé puis restait vide jusqu'à la première actualisation.

### Maintenant

Une page de démarrage est immédiatement affichée :

```text
TrailLink demarre
Recherche GPS...
```

Le code utilisé est :

```cpp
ecran.clearDisplay();
ecran.setTextColor(SSD1306_WHITE);
ecran.setTextSize(1);
ecran.setCursor(18, 20);
ecran.println("TrailLink demarre");
ecran.setCursor(22, 36);
ecran.println("Recherche GPS...");
ecran.display();
```

### Utilité

Cette page permet de voir immédiatement que :

- la carte a démarré ;
- le programme s'exécute ;
- l'écran fonctionne.

---

## 9. Indication GPS corrigée sur l'écran

### Avant

L'écran affichait :

```text
LORA: OK
```

ou :

```text
LORA: NO
```

Pourtant, la radio LoRa n'était pas initialisée.

La valeur affichée correspondait en réalité à la validité du GPS.

### Maintenant

L'écran affiche :

```text
GPS: OK
```

ou :

```text
GPS: NO
```

Le code utilise une variable plus claire :

```cpp
bool gpsValide;
```

### Utilité

L'information présentée à l'utilisateur correspond maintenant à l'état réel du programme.

---

## 10. Véritable niveau de batterie

### Avant

Le programme affichait toujours :

```cpp
mesInfos.percentBatterie = 88.0;
```

La valeur `88 %` était fictive.

### Maintenant

La batterie est interrogée grâce au PMU :

```cpp
int obtenirPourcentageBatterie() {
    if (
        PMU == nullptr ||
        !PMU->isBatteryConnect()
    ) {
        return -1;
    }

    const int pourcentage =
        PMU->getBatteryPercent();

    return (
        pourcentage >= 0 &&
        pourcentage <= 100
    ) ? pourcentage : -1;
}
```

Si une batterie valide est détectée :

```text
Batterie : 74%
```

Si aucune mesure valide n'est disponible :

```text
Batterie : --
```

### Utilité

L'écran n'affiche plus une fausse information.

La valeur est aussi vérifiée pour empêcher l'affichage d'un pourcentage inférieur à 0 ou supérieur à 100.

---

## 11. Diagnostic GPS ajouté

### Avant

Le moniteur série affichait uniquement :

```text
Liaison materielle avec le GPS initialisee.
```

Ce message confirmait seulement l'ouverture du port série. Il ne prouvait pas que le GPS envoyait réellement des informations.

### Maintenant

Le programme compte tous les caractères reçus :

```cpp
donnees.caracteresRecus =
    gps.charsProcessed();
```

Il affiche chaque seconde :

```text
[GPS] Caracteres recus: 1520 | Satellites: 4 | Position valide: oui
```

### Interprétation

Si le compteur augmente :

```text
120
340
580
```

Le GPS est alimenté et communique avec l'ESP32.

Si le compteur reste à zéro :

```text
Caracteres recus: 0
```

Le GPS ne communique pas.

Si le compteur augmente mais que les satellites restent à zéro, la communication fonctionne et il faut tester l'antenne dehors.

### Utilité

Il devient possible de distinguer :

- un GPS non alimenté ;
- un GPS qui communique mais ne voit pas encore les satellites ;
- un GPS qui possède une position valide.

---

## 12. Suppression de l'objet GPS en double

### Avant

Un premier objet GPS était déclaré dans `gps.h` :

```cpp
inline TinyGPSPlus gps;
```

Un deuxième objet portant le même nom était déclaré dans `Affichage.cpp` :

```cpp
TinyGPSPlus gps;
```

Le deuxième objet ne recevait jamais les caractères lus sur le port série.

Une fonction essayait donc de récupérer l'heure depuis un objet GPS vide.

### Maintenant

Le deuxième objet et sa fonction inutilisée ont été supprimés.

Toutes les informations proviennent du même objet :

```cpp
inline TinyGPSPlus gps;
```

Il est alimenté par :

```cpp
gps.encode(SerialGPS.read());
```

### Utilité

Toutes les parties du programme utilisent maintenant la même source de données GPS.

Cela évite des informations différentes ou toujours invalides.

---

## 13. Heure stockée sans `String`

### Avant

L'heure utilisait :

```cpp
String heure;
```

Les objets `String` peuvent réserver et libérer de la mémoire pendant l'exécution.

Après une longue utilisation, ces opérations peuvent fragmenter la mémoire d'un microcontrôleur.

### Maintenant

L'heure utilise une zone fixe :

```cpp
char heure[7];
```

Elle peut contenir :

```text
14:35
```

ou :

```text
CH.SAT
```

### Utilité

La quantité de mémoire utilisée pour l'heure ne change jamais.

Le fonctionnement est donc plus prévisible lors d'une utilisation prolongée.

---

## 14. Types de données plus adaptés

### Avant

Les petites valeurs utilisaient des `int` :

```cpp
int nbSatellites;
int heure;
int minute;
```

Sur l'ESP32, un `int` occupe généralement quatre octets.

### Maintenant

Les valeurs comprises entre 0 et 255 utilisent :

```cpp
uint8_t nbSatellites;
uint8_t heure;
uint8_t minute;
```

Un `uint8_t` occupe un seul octet.

Les états utilisent une énumération :

```cpp
enum EtatAppareil : uint8_t {
    AFF_ETAT_NORMAL = 0,
    AFF_ETAT_PRE_ALERTE,
    AFF_ETAT_SOS
};
```

### Utilité

Les structures sont légèrement plus petites et les états sont plus faciles à comprendre.

---

## 15. Données GPS toujours initialisées

### Avant

La structure était créée ainsi :

```cpp
DonneesGps donnees;
```

Une valeur non remplie aurait pu contenir une donnée inconnue.

### Maintenant

La structure est créée ainsi :

```cpp
DonneesGps donnees{};
```

Toutes les valeurs commencent à zéro ou `false`.

### Utilité

Le programme n'utilise pas accidentellement des valeurs indéterminées.

---

## 16. Textes placés dans la mémoire programme

### Avant

Les messages fixes pouvaient être copiés dans la RAM :

```cpp
Serial.println("[PMU] AXP2101 detecte.");
```

### Maintenant

Les messages utilisent `F()` :

```cpp
Serial.println(
    F("[PMU] AXP2101 detecte.")
);
```

### Utilité

Les textes restent dans la mémoire programme et occupent moins de RAM dynamique.

Cette optimisation laisse davantage de mémoire aux futures fonctions LoRa, SOS et détection de chute.

---

## 17. Constantes matérielles améliorées

### Avant

Les broches étaient définies avec des macros :

```cpp
#define PIN_GPS_RX 34
#define PIN_GPS_TX 12
#define GPS_BAUDRATE 9600
```

### Maintenant

Elles utilisent des constantes typées :

```cpp
constexpr uint8_t PIN_GPS_RX = 34;
constexpr uint8_t PIN_GPS_TX = 12;
constexpr uint32_t GPS_BAUDRATE = 9600;
```

La même méthode est utilisée pour l'écran :

```cpp
constexpr uint8_t PIN_I2C_SDA = 21;
constexpr uint8_t PIN_I2C_SCL = 22;
constexpr uint8_t ADRESSE_OLED = 0x3C;
```

### Utilité

Le compilateur connaît le type exact de chaque valeur et peut mieux vérifier le code.

---

## 18. Vérification de la compilation

La version modifiée a été compilée pour :

```text
ESP32 Dev Module
```

Résultat :

```text
Mémoire programme utilisée :
341 552 octets sur 1 310 720
26 %

Mémoire RAM utilisée :
23 980 octets sur 327 680
7 %

Mémoire RAM restante :
303 700 octets
```

### Conclusion

Le programme tient largement dans la mémoire de l'ESP32.

Il reste suffisamment de mémoire pour ajouter ensuite :

- LoRa ;
- le bouton SOS ;
- le buzzer ;
- la détection de chute.

---

## 19. Fonctions qui ne sont pas encore actives

Les modifications ont surtout fiabilisé :

- l'alimentation ;
- l'écran ;
- le GPS ;
- la batterie ;
- les diagnostics.

Les fonctions suivantes restent à développer ou à activer :

- identification précise de la puce LoRa ;
- émission et réception LoRa ;
- récupération de la latitude et de la longitude dans le programme principal ;
- gestion du bouton utilisateur ;
- déclenchement manuel du SOS ;
- buzzer ;
- accéléromètre ;
- détection automatique de chute ;
- changement automatique entre heure d'été et heure d'hiver.

---

## 20. Résultat général

La version d'origine était un premier prototype capable de compiler, mais plusieurs points pouvaient empêcher son fonctionnement sur la carte réelle :

- PMU imposé ;
- mauvaise sortie d'alimentation du GPS ;
- erreurs silencieuses ;
- écran utilisé même après un échec ;
- batterie fictive ;
- objet GPS en double.

La version actuelle est une base de diagnostic plus sûre :

```text
Démarrage de l'ESP32
        ↓
Détection AXP192 ou AXP2101
        ↓
Alimentation correcte du GPS
        ↓
Radio laissée éteinte
        ↓
Initialisation contrôlée de l'écran
        ↓
Lecture continue du GPS
        ↓
Lecture de la batterie
        ↓
Diagnostic chaque seconde
```

Le programme compile correctement pour la carte `ESP32 Dev Module`. La prochaine validation importante doit être réalisée sur le matériel grâce aux messages du moniteur série.
