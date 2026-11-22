# CFC CAN Discovery Protocol

> Suivi détaillé du projet CFC : dossier `Suivi/ERM_Liaison_DCC` dans l’espace de travail du projet.

Bibliothèque commune des protocoles CAN CFC : découverte générale 11 bits entre ERM réseau CC (S), HUB (H), DISTRIB (D), convertisseurs (C) et LaBox (L), plus les contrats logiques DCC/cutout. L’ERM Liaison DCC gouverne séparément l’association booster-convertisseur : le booster annonce son ID par CAN Booster `0x101` en Exploration, le convertisseur le signale sur CAN Service 29 bits, puis CC et EXCC relaient la validation.

## État

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
https://github.com/prune74/CFC_CAN_Discovery_Protocol.git#v0.6.0
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

- Spécification détaillée et état d’avancement : `Suivi/ERM_Liaison_DCC/PROTOCOLE_CAN_DECOUVERTE_SPECIFICATION.md` dans l’espace de travail du projet.
- `examples/BasicProtocol/BasicProtocol.cpp` : exemple minimal de génération
  et décodage des IDs.
- `examples/ReferenceFrames/` : trames de référence pour le CAN analyseur.

Dans la cible DCC, le CAN Booster standard 11 bits transporte le flux DCC et
les événements cutout sous `0x100`. L’ID `0x101` est réservé à l’annonce
booster en Exploration et ne transporte jamais le flux DCC. Les anciens
formats `0x102`–`0x104` et les anciennes références LaBox standard 11 bits
`0x701` / `0x20`–`0x22` sont supprimés de la cible. Le CAN Service 29 bits
transporte références, supervision, défauts, réglages et association booster.
