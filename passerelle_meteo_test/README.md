# Passerelle météo TrailLink — prototype séparé

Ce dossier contient uniquement le futur programme de la carte qui restera connectée au Wi-Fi.

Il est séparé de `Trailink/`, `emetteur/` et `recepteur/`. Aucun fichier existant de la balise mobile n'a été modifié et rien n'a été téléversé sur une carte.

## Ce que fait le programme

1. Il maintient la connexion Wi-Fi de la passerelle.
2. Il écoute les demandes météo envoyées par LoRa.
3. Il vérifie l'identifiant du protocole et les coordonnées GPS.
4. Il confirme immédiatement la réception de la demande.
5. Il appelle Open-Meteo avec la latitude et la longitude reçues.
6. Il récupère la température, le vent et les rafales.
7. Il renvoie ces valeurs à la balise par LoRa.
8. Il renvoie un statut d'erreur si le Wi-Fi ou le service météo est indisponible.

La passerelle ne déclenche pas elle-même une demande toutes les 15 minutes. Cette fréquence sera gérée plus tard par la balise mobile. La passerelle répond simplement lorsqu'une demande arrive.

## Carte ciblée pour cette première version

Le code vise la T-Beam ESP32 classique visible dans le projet :

- ESP32 classique ;
- radio SX1276 ;
- alimentation AXP192 ;
- fréquence LoRa de 868 MHz ;
- broches radio SCK 5, MISO 19, MOSI 27, CS 18, DIO0 26, RESET 23 et DIO1 33.

Si la seconde carte n'est pas exactement ce modèle, il faudra adapter `Configuration.h` et éventuellement `AlimentationEtRadio.h` avant tout téléversement.

## Bibliothèques nécessaires plus tard

- RadioLib
- ArduinoJson
- XPowersLib
- les bibliothèques Wi-Fi et HTTP incluses avec le paquet ESP32 Arduino

## Avant de compiler ou téléverser

1. Ouvrir `passerelle_meteo_test.ino` dans Arduino IDE.
2. Vérifier que tous les fichiers du dossier apparaissent dans les onglets.
3. Installer RadioLib, ArduinoJson et XPowersLib dans le gestionnaire de bibliothèques.
4. Renseigner le nom et le mot de passe du Wi-Fi dans `Configuration.h`.
5. Confirmer que la passerelle utilise bien une radio SX1276 et un AXP192.
6. Brancher l'antenne LoRa avant d'allumer la radio ou de transmettre.

## Ce qui manque encore pour réaliser le test complet

La balise mobile actuelle n'envoie pas encore de `DemandeMeteo` et ne sait pas encore lire `ReponseMeteo`. Il faudra plus tard copier le même fichier `ProtocoleMeteo.h` dans son projet, puis ajouter l'envoi toutes les 15 minutes ou par bouton.

Le code HTTPS utilise temporairement `setInsecure()` pour faciliter le prototype. Cela chiffre la connexion, mais ne vérifie pas correctement l'identité du serveur. Une version destinée à un usage réel devra utiliser un certificat racine valide.

## Fichiers

- `passerelle_meteo_test.ino` : programme principal ;
- `Configuration.h` : Wi-Fi, paramètres LoRa et broches ;
- `ProtocoleMeteo.h` : format commun des messages radio ;
- `AlimentationEtRadio.h` : alimentation AXP192 et initialisation SX1276.
