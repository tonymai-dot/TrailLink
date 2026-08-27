# TrailLink — Passerelle LoRa vers Internet pour obtenir la météo

## Question posée

> Mais on n'aura pas accès à Internet dans la nature. Le système communique juste par ondes radio. Est-ce faisable de communiquer par ondes radio avec quelque chose qui va chercher sur Internet les informations qu'on lui a données et qui renvoie ensuite l'information ?

## Réponse courte

Oui, c'est faisable.

Il faut créer une **passerelle radio vers Internet**.

La T-Beam du randonneur n'aurait pas besoin d'avoir directement accès à Internet. Elle communiquerait par LoRa avec une deuxième carte placée dans un endroit disposant d'une connexion Internet.

```text
Balise TrailLink mobile
GPS + LoRa
       |
       | demande météo par LoRa
       v
Passerelle TrailLink
LoRa + Wi-Fi/Internet
       |
       | appel de l'API météo
       v
Service météo en ligne
       |
       | température et vent
       v
Passerelle TrailLink
       |
       | réponse par LoRa
       v
Balise mobile
Affichage sur l'écran
```

---

## 1. Exemple concret

La balise mobile commence par obtenir sa position GPS :

```text
Latitude  : 45.9234
Longitude : 6.8691
```

Elle envoie ensuite un petit message par LoRa :

```text
Type de message : demande météo
Identifiant      : 3
Latitude         : 45.9234
Longitude        : 6.8691
```

La passerelle reçoit ce message. Comme elle possède une connexion Internet, elle interroge une API météo avec les coordonnées reçues.

Elle renvoie ensuite à la balise :

```text
Température : 8 °C
Vent estimé : 24 km/h
Rafales     : 38 km/h
Âge donnée  : 2 minutes
```

La balise mobile peut alors afficher :

```text
METEO ESTIMEE

T: 8 C
Vent: 24 km/h
Raf: 38 km/h
```

---

## 2. Quel matériel utiliser pour la passerelle ?

Une deuxième T-Beam peut servir de passerelle :

```text
T-Beam passerelle
├── radio LoRa
├── ESP32
└── connexion Wi-Fi
```

Elle pourrait être placée :

- dans un refuge disposant du Wi-Fi ;
- dans une maison proche de la zone ;
- dans un véhicule avec un partage de connexion ;
- près d'un téléphone partageant sa connexion ;
- sur un point haut disposant d'un accès Internet.

La passerelle resterait allumée et écouterait les demandes LoRa.

---

## 3. Limite principale

Il faut qu'au moins la passerelle dispose d'Internet.

Si la balise mobile ne peut joindre aucune passerelle LoRa, elle ne peut pas obtenir une nouvelle météo en ligne.

```text
Pas de liaison avec la passerelle
                |
                v
Pas de nouvelle météo en ligne
```

Dans ce cas, l'écran devrait afficher :

```text
METEO INDISPONIBLE
```

Une autre possibilité consiste à conserver la dernière réponse reçue :

```text
T: 8C  V: 24km/h
MAJ: il y a 35 min
```

L'utilisateur sait ainsi que la valeur affichée n'est plus récente.

---

## 4. Portée de la liaison LoRa

La portée LoRa dépend beaucoup du terrain :

- elle peut être bonne en visibilité directe et depuis un point haut ;
- elle diminue derrière une montagne ;
- elle diminue dans une vallée encaissée ;
- elle diminue dans une forêt dense ;
- elle diminue entre des bâtiments.

Une antenne adaptée et correctement orientée est indispensable.

Pour une utilisation en montagne, placer la passerelle en hauteur serait beaucoup plus efficace.

Une longue portée annoncée dans des conditions idéales ne garantit donc pas une liaison dans toutes les situations réelles.

---

## 5. Format des messages radio

Il serait possible de définir deux types de messages.

### Demande envoyée par la balise

```cpp
struct DemandeMeteo {
    uint8_t type;
    uint8_t idBalise;
    float latitude;
    float longitude;
};
```

Cette structure contient :

- le type de message ;
- l'identifiant de la balise ;
- la latitude ;
- la longitude.

### Réponse envoyée par la passerelle

```cpp
struct ReponseMeteo {
    uint8_t type;
    uint8_t idBalise;
    int16_t temperatureDixieme;
    uint16_t ventKmh;
    uint16_t rafalesKmh;
    uint32_t dateMiseAJour;
    uint8_t statut;
};
```

Cette structure contient :

- le type de message ;
- l'identifiant de la balise destinataire ;
- la température ;
- la vitesse estimée du vent ;
- la vitesse estimée des rafales ;
- la date ou l'heure de mise à jour ;
- un état indiquant si la demande a réussi.

La température peut être envoyée sous forme d'un entier :

```text
83  = 8,3 °C
-25 = -2,5 °C
```

Cela permet de garder des messages radio petits et simples.

---

## 6. Fréquence des demandes

Il ne faut pas demander la météo chaque seconde.

Une fréquence raisonnable serait :

```text
Première demande lorsque le GPS devient valide
Puis une demande toutes les 15 minutes
Ou une demande manuelle avec un bouton
```

Cela permet de limiter :

- la consommation de batterie ;
- les transmissions radio inutiles ;
- les appels inutiles à l'API ;
- les risques de collision avec d'autres messages radio.

Le bouton utilisateur pourrait être organisé ainsi :

- appui court : changer de page ;
- double appui : demander une nouvelle météo ;
- appui long : déclencher un SOS.

---

## 7. Accusé de réception

La passerelle devrait confirmer qu'elle a reçu la demande.

La balise pourrait afficher :

```text
DEMANDE METEO...
```

La passerelle répondrait d'abord :

```text
DEMANDE RECUE
RECHERCHE METEO...
```

Puis elle enverrait les informations météo.

La balise afficherait finalement :

```text
METEO RECUE
```

Si aucune réponse n'arrive après un délai défini :

```text
PASSERELLE INJOIGNABLE
```

L'accusé de réception est utile car il permet de savoir si le problème vient :

- de la liaison LoRa ;
- de la connexion Internet de la passerelle ;
- de l'API météo.

---

## 8. Solution de secours hors ligne

Une architecture hybride serait plus fiable :

- température locale mesurée directement par un capteur ;
- vent estimé par l'API lorsque la passerelle est accessible ;
- dernière météo conservée lorsque la passerelle devient inaccessible.

Sans passerelle :

```text
T locale : 7,8 C
Vent : indisponible
```

Avec une passerelle :

```text
T météo : 8 C
Vent estimé : 24 km/h
Rafales : 38 km/h
```

La vitesse du vent venant d'une API reste une estimation météorologique. Ce n'est pas une mesure exacte effectuée autour de l'utilisateur.

---

## 9. Programme de la balise mobile

Le programme de la balise doit :

1. obtenir une position GPS valide ;
2. construire une demande météo ;
3. envoyer la demande par LoRa ;
4. attendre un accusé de réception ;
5. attendre la réponse météo ;
6. vérifier que la réponse lui est destinée ;
7. enregistrer les informations reçues ;
8. afficher les informations ;
9. gérer les délais et les erreurs.

Exemple de déroulement :

```text
GPS valide
    |
    v
Envoi de la demande
    |
    v
Attente de la passerelle
    |
    +---- aucune réponse ----> PASSERELLE INJOIGNABLE
    |
    v
Réponse reçue
    |
    v
Affichage météo
```

---

## 10. Programme de la passerelle

Le programme de la passerelle doit :

1. écouter en permanence les messages LoRa ;
2. reconnaître une demande météo ;
3. extraire les coordonnées GPS ;
4. confirmer la réception ;
5. vérifier la connexion Wi-Fi ;
6. interroger l'API météo ;
7. lire la réponse JSON ;
8. créer une réponse radio compacte ;
9. renvoyer cette réponse à la bonne balise ;
10. signaler les erreurs éventuelles.

Exemple :

```text
Réception LoRa
      |
      v
Lecture des coordonnées
      |
      v
Appel de l'API météo
      |
      +---- erreur Internet ----> réponse d'erreur
      |
      v
Création de la réponse
      |
      v
Envoi LoRa vers la balise
```

---

## 11. Utilisation des dossiers actuels du projet

Le projet possède déjà les dossiers :

```text
emetteur/
recepteur/
```

Ils pourraient servir de base :

```text
emetteur   → balise mobile
recepteur  → passerelle connectée à Internet
```

Cependant, ces programmes ne sont pas encore suffisamment complets pour assurer cette fonction.

Il faudra notamment ajouter :

- la récupération de la latitude et de la longitude ;
- le protocole de demande et de réponse ;
- la connexion Wi-Fi de la passerelle ;
- l'appel de l'API météo ;
- les accusés de réception ;
- la gestion des délais ;
- l'affichage des résultats.

---

## 12. Solution recommandée

La solution simple conseillée est :

```text
Une T-Beam mobile sans Internet
                +
Une T-Beam passerelle connectée au Wi-Fi
                +
Une demande météo toutes les 15 minutes
ou déclenchée avec un bouton
```

L'ordre de développement conseillé est :

1. identifier précisément les puces LoRa des deux cartes ;
2. tester un message LoRa simple entre les deux cartes ;
3. ajouter l'identifiant de la balise ;
4. envoyer les coordonnées GPS ;
5. connecter la passerelle au Wi-Fi ;
6. tester l'API météo sur la passerelle ;
7. renvoyer température et vent par LoRa ;
8. afficher le résultat sur la balise ;
9. ajouter les accusés de réception et les erreurs ;
10. ajouter plus tard le SOS.

---

## Conclusion

Le fonctionnement demandé est réalisable : la balise n'a pas besoin d'Internet si elle peut joindre par LoRa une passerelle qui, elle, dispose d'Internet.

La limite principale reste la portée entre la balise et la passerelle. En dehors de cette portée, aucune nouvelle donnée météo en ligne ne pourra être récupérée.

Pour rendre le système plus fiable, la balise devrait conserver la dernière météo reçue et afficher clairement son ancienneté.
