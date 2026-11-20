# CFC CAN Discovery Protocol

Bibliothèque commune du bus CAN de découverte entre ERM (S), HUB (H), DISTRIB (D), convertisseurs (C) et LaBox (L). Les boosters (B) n’y participent pas comme nœuds de découverte ; leur télémétrie peut être relayée en lecture seule par leur convertisseur.

## État

Version de travail `0.3.1`. La spécification v1 fixe le contrat CAN 11 bits,
les types, les opcodes, les délais de présence, le relais de télémétrie et le
repérage TOR des emplacements en Exploration. Elle ajoute l'identité LaBox
distincte du HUB et les trames de référence DCC. La migration coordonnée des
firmwares et leur validation restent à effectuer avant une publication stable
`1.0.0`.

## Utilisation PlatformIO

Dans `platformio.ini`, épingler une version Git explicite, par exemple :

```ini
lib_deps =
https://github.com/prune74/CFC_CAN_Discovery_Protocol.git#v0.3.1
```

Ne pas utiliser une branche flottante pour les firmwares destinés à être déployés.

## API

`src/CfcDiscoveryProtocol.h` fournit les valeurs communes, les codes de types,
les opcodes, les temporisations, les fonctions de conversion/validation des
CAN ID d’arbitrage et le CRC-8 du repérage D→H.

## Documentation et exemples

- `docs/PROTOCOLE_CAN_DECOUVERTE.md` : spécification détaillée.
- `examples/BasicProtocol/BasicProtocol.cpp` : exemple minimal de génération
  et décodage des IDs.
- `examples/ReferenceFrames/` : trames de référence pour le CAN analyseur.

Les CAN locaux DCC `0x100` (blocs), `0x101` (cutout) et `0x102` (télémétrie booster) restent distincts du bus de découverte. Le contrôleur CAN configure explicitement le format standard 11 bits.
