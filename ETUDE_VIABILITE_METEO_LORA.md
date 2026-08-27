# TrailLink — Étude de viabilité de la fonction météo par LoRa

## Objet

Cette note présente une solution permettant à une balise TrailLink utilisée sans Internet dans la nature de recevoir une température et une vitesse de vent estimées.

L'objectif est d'aider le chef de projet à décider s'il est pertinent de réaliser un prototype.

## Résumé de la solution

La balise mobile ne contacte pas directement Internet. Elle transmet sa position GPS par LoRa à une passerelle placée dans une zone couverte par Internet.

```text
Balise mobile
GPS + LoRa
     |
     | coordonnées et demande météo
     v
Passerelle
LoRa + Wi-Fi/Internet
     |
     | requête vers une API météo
     v
Service météo
     |
     | température, vent et rafales
     v
Passerelle
     |
     | réponse LoRa
     v
Écran de la balise mobile
```

Une deuxième T-Beam peut servir de passerelle. Elle doit rester à portée LoRa de la balise et disposer d'une connexion Wi-Fi ou d'un partage de connexion mobile.

## Pourquoi la solution est viable

### Le matériel possède les fonctions nécessaires

La T-Beam regroupe déjà :

- un GPS pour connaître la position de la balise ;
- une radio LoRa pour communiquer à distance ;
- un ESP32 avec Wi-Fi pour la passerelle ;
- un écran pour présenter les informations.

La documentation LilyGO prévoit plusieurs variantes radio de T-Beam et fournit les broches nécessaires au GPS et à LoRa. La référence exacte de la radio devra néanmoins être confirmée sur chaque carte avant le développement.

### Les messages radio peuvent rester très petits

Une demande doit seulement contenir :

```text
Type de message
Identifiant de la balise
Latitude
Longitude
```

La réponse doit seulement contenir :

```text
Température
Vitesse du vent
Rafales
Heure de mise à jour
Code de résultat
```

Ces informations sont adaptées à de petits paquets LoRa.

### L'API peut fournir les informations voulues

Une API telle qu'Open-Meteo accepte une latitude et une longitude et peut retourner la température à 2 mètres, la vitesse du vent à 10 mètres et les rafales.

Pour un prototype non commercial, son API gratuite peut convenir. Elle est toutefois limitée et ne fournit aucune garantie de disponibilité. Une utilisation commerciale nécessiterait de vérifier les conditions et probablement de prévoir une offre adaptée.

### La fonction peut être ajoutée progressivement

Le projet peut être testé sans développer immédiatement le système complet :

1. transmettre un message simple entre deux cartes ;
2. transmettre une position GPS ;
3. connecter la passerelle au Wi-Fi ;
4. interroger l'API ;
5. renvoyer une réponse météo ;
6. afficher la réponse sur la balise.

Chaque étape peut donc être validée séparément.

## Pourquoi la solution peut ne pas être viable

### Elle dépend toujours d'un accès Internet

La balise mobile peut fonctionner sans Internet, mais la passerelle doit obligatoirement en disposer.

Sans passerelle joignable ou sans connexion Internet sur la passerelle, aucune nouvelle météo ne peut être obtenue.

### La portée LoRa n'est pas garantie

La portée dépend fortement :

- du relief ;
- de la présence d'une montagne entre les appareils ;
- de la végétation ;
- de la hauteur et de la position de la passerelle ;
- des antennes ;
- des perturbations radio.

Une balise située dans une vallée encaissée peut ne pas atteindre une passerelle pourtant assez proche géographiquement.

### Le vent fourni n'est pas une mesure locale

La vitesse du vent provenant de l'API est issue d'un modèle météorologique. Elle correspond généralement au vent estimé à 10 mètres de hauteur.

Le vent réellement ressenti près d'un utilisateur peut être différent, notamment en montagne, en forêt ou derrière un obstacle.

L'écran doit donc afficher clairement :

```text
VENT ESTIME
```

et non :

```text
VENT MESURE
```

Ces informations ne doivent pas devenir l'unique base d'une décision de sécurité.

### Le système devient plus complexe

Il faut maintenir deux programmes :

- le programme de la balise mobile ;
- le programme de la passerelle.

Il faut également gérer :

- la connexion Wi-Fi ;
- les erreurs Internet ;
- les délais de réponse ;
- les paquets perdus ;
- les accusés de réception ;
- l'identification de plusieurs balises ;
- les changements éventuels de l'API ;
- les règles applicables aux émissions radio.

### L'API gratuite n'offre pas une disponibilité garantie

Une API gratuite convient pour un prototype, mais elle ne doit pas être considérée comme un service critique garanti.

Pour une version destinée à une utilisation réelle ou commerciale, il faudra prévoir :

- des conditions d'utilisation compatibles ;
- une solution en cas d'indisponibilité ;
- éventuellement un fournisseur ou abonnement professionnel.

## Procédé technique recommandé

### Côté balise mobile

1. Attendre une position GPS valide.
2. Construire une demande météo compacte.
3. Envoyer la demande par LoRa.
4. Attendre un accusé de réception.
5. Attendre la réponse météo pendant un délai limité.
6. Vérifier l'identifiant du destinataire et le type du message.
7. Enregistrer la température, le vent, les rafales et l'heure de mise à jour.
8. Afficher la réponse ou une erreur.

### Côté passerelle

1. Écouter les demandes LoRa.
2. Lire l'identifiant et les coordonnées reçues.
3. Confirmer la réception à la balise.
4. Vérifier la connexion Internet.
5. Interroger l'API météo avec les coordonnées.
6. Contrôler et convertir la réponse JSON.
7. Construire une réponse radio compacte.
8. Renvoyer la réponse à la bonne balise.

### Fréquence conseillée

Il n'est pas nécessaire de demander la météo en continu.

Un fonctionnement simple serait :

```text
Une demande lorsque la position GPS devient valide
Puis une demande toutes les 15 minutes
Et une demande manuelle avec un bouton
```

Cela réduit la consommation, les transmissions inutiles et les appels à l'API.

## Gestion des erreurs recommandée

La balise doit pouvoir afficher au minimum :

```text
DEMANDE METEO...
METEO RECUE
PASSERELLE INJOIGNABLE
INTERNET INDISPONIBLE
SERVICE METEO INDISPONIBLE
DONNEES ANCIENNES
```

La dernière météo reçue peut être conservée, mais son ancienneté doit toujours être visible :

```text
T: 8 C  V: 24 km/h
MAJ: il y a 35 min
```

## Prototype minimal proposé

Le prototype peut être limité à :

- une balise mobile ;
- une passerelle connectée au Wi-Fi ;
- une demande déclenchée manuellement ;
- la température et le vent estimé ;
- un accusé de réception ;
- un affichage de l'ancienneté des données.

Le SOS, la détection de chute et la gestion de plusieurs relais ne doivent pas être ajoutés à ce premier test.

## Tests nécessaires avant validation

### Test radio

- terrain dégagé ;
- forêt ;
- zone urbaine ;
- vallée ou relief ;
- perte puis retour de la liaison.

### Test Internet et API

- connexion Wi-Fi disponible ;
- connexion lente ;
- absence d'Internet ;
- API indisponible ;
- réponse incorrecte ou incomplète.

### Test fonctionnel

- coordonnées GPS valides ;
- demande et réponse destinées à la bonne balise ;
- actualisation toutes les 15 minutes ;
- affichage correct de l'âge des données ;
- conservation de la dernière réponse après une coupure.

## Critères de décision après le prototype

La solution peut être retenue si :

- la liaison radio couvre la zone d'utilisation prévue ;
- la passerelle peut être placée dans un lieu pertinent avec Internet ;
- les réponses arrivent dans un délai acceptable ;
- les erreurs sont clairement indiquées ;
- les utilisateurs comprennent que le vent est une estimation ;
- la consommation reste acceptable ;
- les conditions d'utilisation de l'API correspondent au projet.

La solution doit être abandonnée ou repensée si :

- aucune position de passerelle ne couvre correctement la zone ;
- la connexion Internet de la passerelle est trop instable ;
- l'information météo est interprétée comme une mesure locale de sécurité ;
- la complexité de maintenance dépasse la valeur apportée ;
- le coût d'un service fiable est incompatible avec le projet.

## Estimation qualitative

| Élément | Appréciation |
|---|---|
| Faisabilité technique | Bonne pour un prototype |
| Difficulté logicielle | Moyenne |
| Matériel supplémentaire | Une seconde carte et une connexion Internet |
| Fiabilité en terrain dégagé | À tester, potentiellement bonne |
| Fiabilité en terrain montagneux | Incertaine |
| Précision de la température | Estimation météo |
| Précision du vent local | Limitée |
| Dépendance extérieure | Passerelle, Internet et API |
| Utilisation comme information indicative | Pertinente |
| Utilisation comme système de sécurité | Non recommandée sans autres garanties |

## Recommandation

La solution est suffisamment viable pour justifier un **prototype limité**, à condition qu'une passerelle puisse être installée à portée radio avec un accès Internet.

Le prototype doit servir à mesurer la portée réelle et la fiabilité de la chaîne complète avant toute intégration définitive.

La météo reçue doit rester une information indicative. Le projet ne doit pas présenter la vitesse du vent issue de l'API comme une mesure locale précise ni garantir sa disponibilité.

La décision finale doit être prise après un test entre deux cartes dans les conditions réelles d'utilisation prévues.

## Sources techniques

- Documentation de l'API météo : https://open-meteo.com/en/docs
- Tarification et limites d'utilisation : https://open-meteo.com/en/pricing
- Conditions d'utilisation : https://open-meteo.com/en/terms
- Documentation et exemples LilyGO : https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series
