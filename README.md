# CFC CAN Discovery Protocol

> Suivi détaillé du projet CFC : dossiers `Suivi/Protocoles/CAN_CFC` et `Suivi/Chantiers/Distribution_DCC` dans l’espace de travail du projet.

Bibliothèque commune des protocoles CAN CFC : découverte générale 11 bits entre ERM réseau CC (S), HUB (H), DISTRIB (D), convertisseurs (C) et LaBox (L), plus les contrats logiques DCC/cutout. La carte `ERM_Liaison_CFC` gouverne séparément l’association booster-convertisseur : le booster annonce son ID par CAN Booster `0x101` en Exploration, le convertisseur le signale sur CAN Service 29 bits, puis CC et EXCC relaient la validation.

## État

Version actuelle publiée `0.9.0` (`2fc43f8`) : définit les sources CAN
Service 29 bits et la commande profil/consigne `0xC9`, avec ses relais et
réponses. Les firmwares doivent encore migrer vers cette version.

La version publiée antérieure `0.8.0` a ajouté `LABOX_VOLTAGE_TELEMETRY
(0xC8)` sur CAN Service. Dans cette version, LaBox (source 1) transmet en mV, sur quatre
uint16 little-endian, l’alimentation générale, la sortie 5 V MP1584, la sortie
XL4015 et la tension des rails PROG après DRV8874. `0xFFFF` signale une mesure
invalide. Émission chaque seconde;
la donnée devient périmée après 3 secondes sans mise à jour valide. Cette
télémétrie ne transite pas sur CAN Booster.

## Version 0.9.0 — publiée (`2fc43f8`)

Cette révision définit la répartition des identifiants source du CAN Service
29 bits : CC `1..250`, nouvelle carte éventuelle `251`, CLF `252`,
`ERM_Liaison_CFC` `253`, LaBox `254`, `255` réservé, convertisseurs
`0x0100 + leur ID de découverte C/x`. Les identités de découverte 11 bits
restent inchangées.

Elle ajoute la commande profil/consigne `0xC9` et sa réponse avec le même
opcode, `resp=1`, résultat `0..7`. Le traitement du résultat
`2=destinataire invalide` est défensif : l’ERM rejette un destinataire inconnu
avant émission et les cartes ne répondent pas à un sélecteur invalide.

Le trajet CC/EXCC/Booster utilise CAN Commands `0xE6`/`0xDE` et UART
`0x1A`/`0x1B`. Les résultats UART `0..5` deviennent les résultats CAN
`0,1,3,4,6,7`; le délai EXCC est une nouvelle tentative après 500 ms et le
résultat CAN `5=Booster non confirmé` après 1 000 ms sans réponse applicative.

Chaque destinataire garde en mémoire volatile la dernière transaction, sa
demande exacte et son résultat. Une répétition identique renvoie le résultat
mémorisé; le même numéro avec un contenu différent renvoie CAN `7` sans
appliquer le réglage. Une nouvelle transaction remplace l’entrée; un
redémarrage l’efface. L’ERM attend la fin ou l’expiration avant d’en lancer
une autre.

Cette version est publiée, mais les firmwares ne sont pas encore adaptés à ces
changements.

Version `0.7.0` : ajoute `BOOSTER_RAIL_VOLTAGE (0xC7)` sur CAN Service. La
mesure arrive du Booster par UART `0x19`, puis est relayée sans changement via
CAN Commands `0xDD`. Les deux trames CAN font 8 octets : tension des rails en
mV (uint16 little-endian), validité (`1` valide, `0` invalide), puis cinq
octets réservés à zéro. La mesure est transmise chaque seconde et devient
périmée après 3 secondes sans nouvelle valeur valide.

Version `0.6.0` : ajoute la commande CAN Service `0xC6` pour relire les
corrections demi-bit sauvegardées dans un Booster et le type UART `0x18`
correspondant. La lecture est sans effet sur le réglage.

Version `0.5.0` : ajoute la découverte/association Booster, avec CAN Booster
`0x101`, CAN Service `0xBB`/`0xC4`/`0xC5`, CAN Commands `0xD3`/`0xED`–`0xEF`
et UART `0x15`–`0x17`. Le CAN Booster `0x101` n’est plus le cutout historique;
Booster et convertisseur doivent être mis à jour ensemble, DCC arrêté. Les
relais et traitements correspondants sont intégrés et compilent; le bus réel
reste à vérifier. La durée réelle du cutout `0xB8` demeure invalide jusqu’à
sa mesure à la sortie HUB LaBox.

## Utilisation PlatformIO

Dans `platformio.ini`, épingler une version Git explicite, par exemple :

```ini
lib_deps =
https://github.com/prune74/CFC_CAN_Discovery_Protocol.git#v0.9.0
```

Ne pas utiliser une branche flottante pour les firmwares destinés à être déployés.

## API

`src/CfcDiscoveryProtocol.h` fournit les valeurs communes de découverte,
opcodes, temporisations, validation des CAN ID d’arbitrage et CRC-8 du repérage
D→H. `src/CfcDccProtocol.h` fixe les commandes DCC, identifiants, drapeaux,
causes et dispositions des charges utiles CAN Booster et CAN Service. Le format
des trames UART et les types communs sont dans
`Gestion_Canton_Protocol/src/UartProtocol.h`.

## Documentation et exemples

- Spécification détaillée et état d’avancement : `Suivi/Protocoles/CAN_CFC/PROTOCOLE_CAN_DECOUVERTE_SPECIFICATION.md` dans l’espace de travail du projet.
- `examples/BasicProtocol/BasicProtocol.cpp` : exemple minimal de génération
  et décodage des IDs.
- `examples/ReferenceFrames/` : trames de référence pour le CAN analyseur.

Dans la cible DCC, le CAN Booster standard 11 bits transporte le flux DCC et
les événements cutout sous `0x100`. L’ID `0x101` est réservé à l’annonce
booster en Exploration et ne transporte jamais le flux DCC. Les anciens
formats `0x102`–`0x104` et les anciennes références LaBox standard 11 bits
`0x701` / `0x20`–`0x22` sont supprimés de la cible. Le CAN Service 29 bits
transporte références, supervision, défauts, réglages et association booster.
