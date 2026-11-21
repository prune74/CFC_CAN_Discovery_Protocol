# CFC CAN Discovery Protocol

> Suivi détaillé du projet CFC : dossier `Suivi/ERM_Liaison_DCC` dans l’espace de travail du projet.

Bibliothèque commune des protocoles CAN CFC : découverte 11 bits entre ERM (S), HUB (H), DISTRIB (D), convertisseurs (C) et LaBox (L), plus les contrats logiques DCC/Cutout et les messages de référence/supervision 29 bits. Les boosters (B) ne participent pas comme nœuds de découverte; ils reçoivent le flux DCC sur le CAN Booster et leurs diagnostics par UART puis CAN Service.

## État

Version de travail `0.4.0`. Elle conserve la découverte CAN 11 bits v1 et
formalise les messages du projet DCC : flux CAN Booster `0x100`, supervision et
références CAN Service 29 bits `0xB6`–`0xBA`, et types UART `0x06`, `0x12`–`0x14`.
Ces contrats sont publiés dans la bibliothèque; l’intégration coordonnée aux
firmwares et les validations sur matériel restent à effectuer. La valeur réelle
du cutout `0xB8` demeure invalide jusqu’à sa mesure à la sortie HUB LaBox.

## Utilisation PlatformIO

Dans `platformio.ini`, épingler une version Git explicite, par exemple :

```ini
lib_deps =
https://github.com/prune74/CFC_CAN_Discovery_Protocol.git#v0.4.0
```

Ne pas utiliser une branche flottante pour les firmwares destinés à être déployés.

## API

`src/CfcDiscoveryProtocol.h` fournit les valeurs communes de découverte,
opcodes, temporisations, validation des CAN ID d’arbitrage et CRC-8 du repérage
D→H. `src/CfcDccProtocol.h` fixe les commandes DCC, identifiants, drapeaux,
causes et dispositions des charges utiles CAN Booster, CAN Service et UART.

## Documentation et exemples

- Spécification détaillée et état d’avancement : `Suivi/ERM_Liaison_DCC/PROTOCOLE_CAN_DECOUVERTE_SPECIFICATION.md` dans l’espace de travail du projet.
- `examples/BasicProtocol/BasicProtocol.cpp` : exemple minimal de génération
  et décodage des IDs.
- `examples/ReferenceFrames/` : trames de référence pour le CAN analyseur.

Dans la cible DCC, le CAN Booster standard 11 bits transporte uniquement le
flux DCC et les événements cutout sous `0x100`. Les anciens IDs locaux
`0x101`–`0x104` et les anciennes références LaBox standard 11 bits `0x701` /
`0x20`–`0x22` ne font pas partie de la cible; ils restent dans les firmwares
actuels jusqu’à leur migration. Le CAN Service 29 bits transporte références,
supervision, défauts et réglages.
