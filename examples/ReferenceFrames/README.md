# Trames de référence

Tous les IDs indiqués sont des identifiants standard CAN 2.0A sur 11 bits.
Les octets sont affichés dans l’ordre du DLC, en hexadécimal.

## Présence de l’ERM S1 en mode Exploration, avec HUB

| CAN ID | DLC | Données |
|---:|---:|---|
| `0x501` | 8 | `01 01 00 00 01 01 01 01` |

Décodage : `HELLO`, version majeure 1, transaction 0, TYPE S=1, ID=1,
mode Exploration=1, topologie avec HUB=1.

## Télémétrie du booster B42 relayée par le convertisseur C11

| CAN ID | DLC | Données |
|---:|---:|---|
| `0x68B` | 6 | `2A 05 0C B4 00 0B` |

Décodage : canton 42, TYPE B=5, courant `0x0C` = 120 mA, tension `0xB4` =
18,0 V, état OK=0, source C11. La copie DLC 6 ne revient jamais sur le CAN DCC.

## Repères de calcul

`CAN ID = 0x500 + (code de classe × 0x80) + ID logique`

- S1 : `0x501`
- H1 : `0x581`
- D1 : `0x601`
- C11 : `0x68B`
- C88 : `0x6D8`

L’adresse `0x7F` est réservée à une transaction séquentielle pour une carte
non attribuée ; elle n’est jamais enregistrée dans l’inventaire.
