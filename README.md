# CFC CAN Discovery Protocol

Bibliothèque commune du bus CAN de découverte entre ERM (S), HUB (H), DISTRIB (D) et convertisseurs (C). Les boosters (B) n’y participent pas comme nœuds de découverte ; leur télémétrie peut être relayée en lecture seule par leur convertisseur.

## État

Version de départ `0.1.0`. Le contrat 11 bits, les types, les opcodes retenus, le relais de télémétrie et les temporisations validées sont regroupés ici. Les points encore marqués « à trancher » dans la spécification ne sont pas des garanties de compatibilité de production. La version `1.0.0` sera publiée après fermeture de ces points et migration coordonnée des firmwares.

## Utilisation PlatformIO

Dans `platformio.ini`, épingler une version Git explicite, par exemple :

```ini
lib_deps =
    https://github.com/prune74/CFC_CAN_Discovery_Protocol.git#v0.1.0
```

Ne pas utiliser une branche flottante pour les firmwares destinés à être déployés.

## API

`src/CfcDiscoveryProtocol.h` fournit les valeurs communes, les codes de types, les opcodes, les temporisations et les fonctions de conversion/validation des CAN ID d’arbitrage.

## Documentation et exemples

- `docs/PROTOCOLE_CAN_DECOUVERTE.md` : spécification détaillée.
- `examples/BasicProtocol/BasicProtocol.cpp` : exemple minimal de génération
  et décodage des IDs.
- `examples/ReferenceFrames/` : trames de référence pour le CAN analyseur.

Les CAN locaux DCC `0x100` (blocs), `0x101` (cutout) et `0x102` (télémétrie booster) restent distincts du bus de découverte. Le contrôleur CAN configure explicitement le format standard 11 bits.
