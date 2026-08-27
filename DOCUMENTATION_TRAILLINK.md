# TrailLink — Explication simple du programme

## 1. À quoi sert ce programme ?

Ce programme est installé sur une carte **LilyGO T-Beam** équipée d'un ESP32.

Pour le moment, il permet de tester et d'utiliser :

- le gestionnaire d'alimentation de la carte ;
- l'écran OLED ;
- le GPS ;
- la mesure de la batterie ;
- le moniteur série d'Arduino IDE.

Le programme affiche l'heure GPS, le nombre de satellites, l'état du GPS et le niveau de batterie.

Les fonctions suivantes sont prévues, mais ne sont pas encore actives :

- l'envoi et la réception LoRa ;
- le bouton SOS ;
- la détection de chute ;
- le buzzer ;
- l'envoi des coordonnées à une deuxième carte.

---

## 2. Résumé très simple

Quand la carte démarre, elle effectue ces opérations dans l'ordre :

```text
1. Elle démarre la communication avec l'ordinateur
2. Elle cherche le composant qui gère l'alimentation
3. Elle alimente le GPS et les autres composants
4. Elle démarre l'écran OLED
5. Elle démarre le GPS
6. Elle lit continuellement les informations GPS
7. Elle actualise l'écran une fois par seconde
```

---

## 3. Les fichiers du programme

Le programme se trouve dans le dossier `Trailink` :

```text
Trailink/
├── Trailink.ino
├── Affichage.h
├── Affichage.cpp
├── gps.h
└── LoraLink.h
```

### `Trailink.ino`

C'est le fichier principal. Il décide dans quel ordre les actions sont exécutées.

### `Affichage.h`

Il décrit les informations que l'écran peut afficher.

### `Affichage.cpp`

Il gère :

- l'alimentation de la carte ;
- la détection du modèle de composant d'alimentation ;
- le démarrage de l'écran ;
- le contenu dessiné sur l'écran ;
- la lecture de la batterie.

### `gps.h`

Il démarre le GPS, lit ses messages et récupère l'heure et le nombre de satellites.

### `LoraLink.h`

Il contient une première version du code LoRa. Ce fichier n'est pas encore utilisé par le programme principal.

---

## 4. Démarrage du programme

Sur Arduino, la fonction `setup()` est exécutée une seule fois lorsque :

- la carte est allumée ;
- le programme vient d'être téléversé ;
- le bouton RESET est pressé.

Voici une version simplifiée :

```cpp
void setup() {
    Serial.begin(115200);

    affichageDisponible = initialiserAffichage();
    initialiserGps();

    mesInfos.percentBatterie = obtenirPourcentageBatterie();
    mesInfos.gpsValide = false;
    mesInfos.etatLappareil = AFF_ETAT_NORMAL;
}
```

### Communication avec l'ordinateur

```cpp
Serial.begin(115200);
```

Cette ligne permet à l'ESP32 d'envoyer du texte vers le moniteur série.

Dans Arduino IDE, le moniteur série doit être réglé sur :

```text
115200 baud
```

Au démarrage, le programme affiche :

```text
=== Demarrage TrailLink ===
```

---

## 5. Gestion de l'alimentation

La T-Beam possède un composant appelé **PMU**. Il distribue l'électricité au GPS, à la radio et aux autres composants.

Selon la version de la T-Beam, ce PMU peut être :

- un `AXP192` ;
- un `AXP2101`.

Le programme essaie automatiquement les deux modèles.

### Première tentative : AXP2101

```cpp
PMU = new XPowersAXP2101(Wire, PIN_I2C_SDA, PIN_I2C_SCL);

if (PMU->init()) {
    Serial.println(F("[PMU] AXP2101 detecte."));
}
```

Si le composant répond, le programme sait que la carte utilise un AXP2101.

### Deuxième tentative : AXP192

Si le premier essai échoue, le programme essaie :

```cpp
PMU = new XPowersAXP192(Wire, PIN_I2C_SDA, PIN_I2C_SCL);

if (PMU->init()) {
    Serial.println(F("[PMU] AXP192 detecte."));
}
```

### Si aucun modèle ne répond

Le moniteur série affiche :

```text
[ERREUR] Aucun PMU AXP192/AXP2101 detecte.
```

Dans ce cas, le programme tente quand même de démarrer l'écran, mais le GPS peut ne pas être alimenté.

### Sorties utilisées avec l'AXP2101

Pour une T-Beam AXP2101 :

```text
ALDO2 → radio LoRa, désactivée pour le moment
ALDO3 → GPS, activé
```

Les deux sorties sont réglées sur 3,3 volts.

### Sorties utilisées avec l'AXP192

Pour une T-Beam AXP192 :

```text
DCDC1 → écran
LDO2  → radio LoRa, désactivée pour le moment
LDO3  → GPS
```

Ces sorties sont également réglées sur 3,3 volts.

Le programme n'alimente pas encore la radio, puisqu'elle n'est pas utilisée par le sketch principal. Cela réduit la consommation et évite d'activer inutilement un composant qui n'est pas encore configuré.

Lorsque LoRa sera ajouté au programme, il faudra réactiver `ALDO2` ou `LDO2` juste avant l'initialisation de la radio.

---

## 6. Démarrage de l'écran

L'écran communique avec l'ESP32 grâce à deux fils logiques :

```text
SDA → GPIO 21
SCL → GPIO 22
```

Ces deux fils constituent le bus **I2C**.

Le programme recherche :

```text
Type d'écran : SSD1306
Taille        : 128 × 64 pixels
Adresse I2C   : 0x3C
```

Le code principal est :

```cpp
if (!ecran.begin(SSD1306_SWITCHCAPVCC, ADRESSE_OLED)) {
    Serial.println(F(
        "[ERREUR] Ecran OLED introuvable a l'adresse 0x3C."
    ));
    return false;
}
```

Si l'écran fonctionne, il affiche d'abord :

```text
TrailLink demarre
Recherche GPS...
```

Le moniteur série affiche :

```text
[OLED] Ecran initialise avec succes.
```

### Si l'écran ne fonctionne pas

Le programme indique :

```text
[ERREUR] Ecran OLED introuvable a l'adresse 0x3C.
```

Le GPS continue alors de fonctionner et ses informations restent visibles dans le moniteur série.

Cela évite que tout le programme s'arrête uniquement à cause de l'écran.

---

## 7. Démarrage du GPS

Le GPS communique avec l'ESP32 par un port série séparé du port USB.

Les réglages sont :

```text
Réception GPS : GPIO 34
Envoi au GPS  : GPIO 12
Vitesse       : 9600 bauds
```

Le code est :

```cpp
SerialGPS.begin(
    GPS_BAUDRATE,
    SERIAL_8N1,
    PIN_GPS_RX,
    PIN_GPS_TX
);
```

Le moniteur série affiche ensuite :

```text
[GPS] Liaison serie initialisee sur RX=34, TX=12, 9600 bauds.
```

Ce message signifie que l'ESP32 a ouvert la communication. Il ne garantit pas encore que le GPS envoie des données.

---

## 8. Lecture continue du GPS

Après le démarrage, Arduino répète continuellement la fonction `loop()`.

La première action est toujours :

```cpp
mettreAJourGps();
```

Cette fonction lit tous les caractères envoyés par le GPS :

```cpp
while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
}
```

Explication :

1. `SerialGPS.available()` vérifie si des données sont disponibles.
2. `SerialGPS.read()` lit un caractère.
3. `gps.encode()` donne ce caractère à TinyGPSPlus.
4. TinyGPSPlus reconstruit les informations GPS.

Le GPS envoie ses informations sous forme de phrases appelées **NMEA**.

La bibliothèque TinyGPSPlus transforme ces phrases en informations faciles à utiliser :

- heure ;
- position ;
- satellites ;
- validité du signal.

---

## 9. Pourquoi il n'y a pas de `delay(1000)` dans la boucle

Le GPS envoie continuellement des données. Le programme doit donc les lire aussi souvent que possible.

Au lieu de bloquer le programme avec :

```cpp
delay(1000);
```

le programme utilise :

```cpp
if (millis() - dernierAffichage >= 1000) {
    dernierAffichage = millis();
}
```

Cela signifie :

> Si une seconde s'est écoulée depuis le dernier affichage, actualiser l'écran.

Pendant cette seconde, l'ESP32 continue de lire le GPS.

Cette méthode est plus adaptée à un programme qui devra ensuite gérer en même temps :

- le GPS ;
- LoRa ;
- les boutons ;
- le buzzer ;
- l'accéléromètre.

---

## 10. Informations récupérées du GPS

Les données sont regroupées dans cette structure :

```cpp
struct DonneesGps {
    uint8_t nbSatellites;
    uint8_t heure;
    uint8_t minute;
    bool aSignalValide;
    unsigned long caracteresRecus;
};
```

### `nbSatellites`

Nombre de satellites actuellement détectés.

### `heure` et `minute`

Heure reçue depuis les satellites.

### `aSignalValide`

Cette valeur est vraie uniquement si le GPS possède :

- une position valide ;
- une heure valide.

Le code utilise :

```cpp
donnees.aSignalValide =
    gps.location.isValid() && gps.time.isValid();
```

Le symbole `&&` signifie « ET ».

### `caracteresRecus`

Nombre total de caractères reçus du GPS :

```cpp
donnees.caracteresRecus = gps.charsProcessed();
```

Ce compteur est très utile pour trouver une panne.

---

## 11. Comprendre les messages GPS

Chaque seconde, le moniteur série affiche par exemple :

```text
[GPS] Caracteres recus: 1520 | Satellites: 4 | Position valide: oui
```

### Cas 1 : le compteur augmente

```text
Caracteres recus: 120
Caracteres recus: 340
Caracteres recus: 580
```

Le GPS est alimenté et communique avec l'ESP32.

### Cas 2 : le compteur augmente, mais aucun satellite

```text
Caracteres recus: 3500 | Satellites: 0
```

La communication fonctionne, mais le GPS ne voit pas encore le ciel.

Il faut :

- sortir la carte dehors ;
- orienter l'antenne GPS vers le ciel ;
- attendre plusieurs minutes lors du premier démarrage.

### Cas 3 : le compteur reste toujours à zéro

```text
Caracteres recus: 0 | Satellites: 0
```

Le GPS n'envoie aucune information.

Les causes possibles sont :

- GPS non alimenté ;
- mauvaise version de carte ;
- mauvaises broches ;
- problème de connexion ;
- vitesse GPS différente de 9600 bauds.

### Cas 4 : satellites présents, mais position non valide

Le GPS a commencé son acquisition, mais il n'a pas encore assez d'informations pour fournir une position fiable.

---

## 12. Heure affichée

Le GPS fournit l'heure UTC.

Le programme ajoute actuellement deux heures :

```cpp
h += 2;
```

Cela correspond à l'heure d'été en France.

Exemple :

```text
Heure GPS UTC : 12:30
Heure affichée : 14:30
```

Limite actuelle :

- été : ajouter 2 heures ;
- hiver : ajouter seulement 1 heure.

Le changement été/hiver n'est pas encore automatique.

---

## 13. Lecture de la batterie

Le programme demande le niveau de batterie au PMU :

```cpp
int obtenirPourcentageBatterie() {
    if (PMU == nullptr || !PMU->isBatteryConnect()) {
        return -1;
    }

    const int pourcentage = PMU->getBatteryPercent();

    return (pourcentage >= 0 && pourcentage <= 100)
        ? pourcentage
        : -1;
}
```

### Si une batterie est détectée

La fonction renvoie une valeur entre 0 et 100.

Exemple :

```text
Batterie : 74%
```

### Si aucune batterie n'est détectée

La fonction renvoie `-1`.

L'écran affiche alors :

```text
Batterie : --
```

Cela évite d'afficher un faux pourcentage.

---

## 14. Contenu de l'écran

### Pendant la recherche GPS

```text
CH.SAT              GPS: NO
---------------------------
Satellites : 0

Batterie   : 74%

Statut     : Securise
```

### Lorsque le GPS est valide

```text
14:35               GPS: OK
---------------------------
Satellites : 6

Batterie   : 74%

Statut     : Securise
```

`CH.SAT` signifie « recherche des satellites ».

---

## 15. Les trois états prévus

Le programme possède trois états :

```cpp
enum EtatAppareil : uint8_t {
    AFF_ETAT_NORMAL = 0,
    AFF_ETAT_PRE_ALERTE,
    AFF_ETAT_SOS
};
```

### État normal

```cpp
AFF_ETAT_NORMAL
```

L'écran affiche les informations GPS et la batterie.

### Pré-alerte

```cpp
AFF_ETAT_PRE_ALERTE
```

L'écran est prévu pour afficher :

```text
ATTENTION
Chute detectee !
```

### SOS

```cpp
AFF_ETAT_SOS
```

L'écran est prévu pour afficher :

```text
SOS
ALERTE EN COURS...
```

Pour le moment, le programme reste toujours en état normal. Aucun bouton ou capteur ne déclenche encore les deux autres états.

---

## 16. Optimisations déjà réalisées

### Utilisation limitée de la mémoire

Les petits nombres utilisent `uint8_t`, qui occupe un seul octet :

```cpp
uint8_t nbSatellites;
uint8_t heure;
uint8_t minute;
```

### Heure stockée dans une zone fixe

L'heure est stockée dans :

```cpp
char heure[7];
```

Cela évite de créer et supprimer continuellement des objets `String` dans la boucle.

Sur un microcontrôleur qui fonctionne longtemps, cela réduit le risque de fragmentation de la mémoire.

### Textes placés dans la mémoire programme

Exemple :

```cpp
Serial.println(F("[PMU] AXP2101 detecte."));
```

Le `F()` garde le texte dans la mémoire programme au lieu de le copier en RAM.

### Données toujours initialisées

```cpp
DonneesGps donnees{};
```

Les valeurs commencent toutes à zéro. Cela évite d'utiliser accidentellement des données inconnues.

### Écran utilisé seulement s'il fonctionne

```cpp
if (affichageDisponible) {
    rafraichirEcran(mesInfos);
}
```

Si l'écran ne démarre pas, le reste du programme continue.

---

## 17. Mémoire utilisée

La compilation a été validée pour :

```text
Carte Arduino : ESP32 Dev Module
```

Résultat :

```text
Mémoire programme utilisée : 341 584 octets sur 1 310 720
Pourcentage utilisé        : 26 %

Mémoire RAM utilisée       : 23 980 octets sur 327 680
Pourcentage utilisé        : 7 %

RAM encore disponible      : 303 700 octets
```

Le programme tient donc largement dans l'ESP32.

Il reste suffisamment de mémoire pour ajouter plus tard LoRa, les boutons et la logique SOS.

---

## 18. Ce qui n'est pas encore utilisé

### LoRa

Le fichier `LoraLink.h` existe, mais il n'est pas inclus dans `Trailink.ino`.

Avant de l'activer, il faut confirmer la puce radio :

- SX1276 ;
- SX1278 ;
- ou SX1262.

La mauvaise classe RadioLib ou de mauvaises broches empêcheraient la radio de fonctionner.

Important : une antenne adaptée doit toujours être connectée avant une émission LoRa.

### Boutons

La carte possède trois boutons :

- alimentation ;
- bouton utilisateur IO38 ;
- reset.

Le programme ne lit actuellement aucun bouton.

### Détection de chute

Le MPU6050 n'est pas encore utilisé par le programme actif.

### Buzzer

Aucune broche de buzzer n'est encore définie.

---

## 19. Messages normaux attendus

Exemple avec un AXP2101 :

```text
=== Demarrage TrailLink ===
[PMU] AXP2101 detecte.
[OLED] Ecran initialise avec succes.
[GPS] Liaison serie initialisee sur RX=34, TX=12, 9600 bauds.
[GPS] Caracteres recus: 240 | Satellites: 0 | Position valide: non
```

Exemple avec un AXP192 :

```text
=== Demarrage TrailLink ===
[PMU] AXP192 detecte.
[OLED] Ecran initialise avec succes.
[GPS] Liaison serie initialisee sur RX=34, TX=12, 9600 bauds.
```

---

## 20. Ordre conseillé pour continuer le projet

Il est préférable d'ajouter les fonctionnalités une par une :

```text
Étape 1 : vérifier l'écran
Étape 2 : vérifier le compteur de caractères GPS
Étape 3 : obtenir une position GPS dehors
Étape 4 : vérifier le vrai niveau de batterie
Étape 5 : identifier et tester la radio LoRa
Étape 6 : ajouter le bouton SOS
Étape 7 : ajouter le buzzer
Étape 8 : ajouter la détection de chute
Étape 9 : tester le système complet
```

Cette méthode permet de savoir immédiatement quelle nouvelle fonction provoque un problème.

---

## 21. Tableau d'avancement

| Fonction | État |
|---|---|
| Compilation pour ESP32 Dev Module | Validée |
| Détection AXP192 | Réalisée |
| Détection AXP2101 | Réalisée |
| Alimentation GPS | Réalisée |
| Initialisation OLED | Réalisée |
| Lecture continue du GPS | Réalisée |
| Diagnostic GPS | Réalisé |
| Affichage du nombre de satellites | Réalisé |
| Affichage de l'heure | Réalisé |
| Lecture de la batterie | Réalisée, à tester sur la carte |
| Passage automatique été/hiver | À faire |
| Latitude et longitude utilisables | À faire |
| Radio LoRa | À identifier et tester |
| Bouton SOS | À faire |
| Buzzer | À faire |
| Détection de chute | À faire |
| Envoi d'un SOS complet | À faire |

---

## 22. Informations matérielles à compléter

Ces informations seront nécessaires pour continuer correctement :

```text
Version écrite sur la carte :

PMU détecté dans le moniteur série :

Référence écrite sur la puce radio :

Fréquence LoRa écrite sur l'étiquette :

Modèle du GPS :

Pourcentage de batterie affiché :

Nombre de caractères GPS après 30 secondes :

Nombre de satellites dehors :
```

---

## 23. Nouvelles fonctions souhaitées

Tu peux ajouter tes idées ici :

```text
- 
- 
- 
```

Questions à décider :

```text
Durée d'appui pour déclencher le SOS :

Durée avant le SOS automatique après une chute :

Méthode utilisée pour annuler un SOS :

Intervalle entre deux messages LoRa :

Informations à envoyer dans un message :

Informations supplémentaires à afficher sur l'écran :

Comportement lorsque la batterie est faible :
```

---

## 24. Conclusion

Le programme actuel est une base de test stable pour l'écran, le GPS, l'alimentation et la batterie.

Il est volontairement organisé de façon à ce qu'une panne d'écran n'empêche pas le diagnostic du GPS.

La compilation montre que l'ESP32 possède encore beaucoup de mémoire disponible. Le prochain élément logique à tester sera la radio LoRa, mais seulement après avoir identifié précisément sa référence sur la carte.
