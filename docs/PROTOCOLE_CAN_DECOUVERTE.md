# Protocole CAN de découverte CFC

**Statut : spécification de travail v1 ; les trames et temporisations ci-dessous
sont retenues, sous réserve de validation finale avant publication de la
bibliothèque commune.** Ce document fixe le contrat de communication entre
l’ERM (S), le HUB (H), les DISTRIB (D) et les convertisseurs (C). Les boosters
(B) ne participent pas à la découverte ; leur télémétrie est seulement relayée
en lecture seule.

La bibliothèque partagée GitHub sera l’implémentation de référence. Les
firmwares ne devront pas recopier localement les IDs, opcodes ou fonctions de
codage.

## 1. Bus et règles de base

- Réseau CAN de découverte : CAN 2.0A, identifiants standard 11 bits,
  500 kbit/s.
- ERM, HUB et DISTRIB utilisent leur contrôleur TWAI.
- Chaque convertisseur utilise son MCP2515 pour rejoindre ce bus. Son TWAI
  local continue à transporter le DCC et la télémétrie des boosters de son
  segment.
- Le bus DCC local et le bus de découverte sont deux segments distincts. Les
  blocs DCC `0x100` et cutouts `0x101` ne sont jamais relayés sur le CAN de
  découverte.
- Les nombres multi-octets de l’en-tête de contrôle sont little-endian.
- Les nœuds ignorent une trame de contrôle dont l’ID CAN, le DLC, la version ou
  les champs obligatoires sont invalides. Ils ne répondent pas à une trame
  invalide.
- Les trames de contrôle sont idempotentes : répéter une attribution identique
  ne doit pas changer son résultat.
- Chaque émetteur possède un identifiant d’arbitrage CAN unique dérivé de son
  type et de son ID. Le CAN de découverte n’utilise pas un identifiant
  d’arbitrage partagé par toutes les cartes, afin d’éviter les collisions de
  contenus différents émis simultanément avec le même ID CAN.

## 2. Registre des types et des identités

| Valeur TYPE | Lettre | Carte | Identité |
|---:|:---:|---|---|
| 1 | S | ERM | Unique, ID 1 |
| 2 | H | HUB | Unique, ID 1 s’il est installé |
| 3 | D | DISTRIB | ID égal à la sortie HUB, 1..8 ; ID 1 en mode direct |
| 4 | C | Convertisseur | `ID_D × 10 + sortie locale`, 11..88 selon l’ID D |
| 5 | B | Booster | ID de canton 1..252 attribué par le CC ; non géré par S |
| 0, 6..254 | — | Réservé | Invalide dans cette version |
| 255 | — | Non attribué | Marqueur d’absence d’identité, jamais un TYPE valide |

Les IDs D et C sont attachés aux emplacements physiques. Une MAC nouvelle sur
un emplacement connu est un remplacement potentiel, pas une autorisation
automatique de réattribuer. Les IDs des emplacements absents restent réservés.

La taille du champ ID par type est d’un octet. L’ID d’un booster est donc
limité à 252, conformément à la plage existante.

## 3. Identifiants CAN

Le CAN de découverte utilise un identifiant d’arbitrage propre à l’émetteur.
Les messages de contrôle conservent leur DLC 8 et leur opcode en `data[0]` ;
les télémétries conservent leur DLC 5 ou 6 et leur format de données. Le
récepteur reconnaît leur catégorie par DLC. Pour une carte attribuée, il vérifie
la cohérence entre la source déduite du CAN ID et le TYPE/ID du contenu. Une
carte non attribuée peut utiliser temporairement `ID=0x7F` dans sa classe,
uniquement pendant une transaction sérialisée.

| Type source | Code de classe | Formule CAN ID de découverte | Exemple |
|---:|---:|---|---:|
| S | 0 | `0x500 + (classe × 0x80) + ID` | S1 → `0x501` |
| H | 1 | `0x500 + (classe × 0x80) + ID` | H1 → `0x581` |
| D | 2 | `0x500 + (classe × 0x80) + ID` | D1 → `0x601` |
| C | 3 | `0x500 + (classe × 0x80) + ID` | C11 → `0x68B`, C88 → `0x6D8` |

Les CAN ID sont standard sur 11 bits. Un nœud sans ID attribué utilise
temporairement l’adresse `0x7F` dans son code de classe, uniquement pour une
transaction sérialisée ; un seul nœud non attribué peut émettre à la fois.
L’adresse temporaire n’est jamais enregistrée comme identité. Une carte non
attribuée ne diffuse pas de heartbeat spontanément ; elle répond uniquement à
une requête ciblée ou à sa demande locale autorisée.

La formule et les exemples de ce tableau sont validés. Chaque carte attribuée
utilise son propre CAN ID d’arbitrage sur le bus de découverte.

Le CAN DCC local conserve ses IDs physiques `0x100`, `0x101` et `0x102` sans
changement. Sur le CAN de découverte, le contenu télémétrique historiquement
appelé format `0x102` est transmis sous le CAN ID unique de son émetteur. Le
format de contrôle historiquement associé à `0x103` utilise lui aussi l’ID
d’arbitrage unique du nœud. La migration doit donc modifier les filtres CAN
de supervision, mais n’affecte pas les CAN DCC locaux.

## 4. Format des trames de contrôle (famille historique `0x103`)

Toutes les trames de contrôle utilisent le même en-tête et un DLC de 8. Leur
CAN ID physique est celui de l’émetteur défini à la section 3 ; `0x103` désigne
ici l’ancienne famille d’opcodes, pas un CAN ID partagé.

| Octet | Signification |
|---:|---|
| `data[0]` | Opcode |
| `data[1]` | Version majeure du protocole, initialement `1` |
| `data[2..3]` | Numéro de transaction little-endian ; `0` pour les annonces périodiques non sollicitées |
| `data[4..7]` | Charge utile de 4 octets, définie par opcode |

Les transactions initiées par S utilisent un numéro non nul. Toute réponse
copie le numéro reçu ; une réponse avec un autre numéro est ignorée. Le
numéro est choisi aléatoirement à l’ouverture d’une session et incrémenté
pour chaque transaction. Il sert à corréler les échanges, pas à authentifier
un nœud. Les octets de charge utile non utilisés valent zéro. Une version
majeure inconnue entraîne un refus explicite si le format permet de la lire ;
les trames mal formées sont ignorées.

### Annonce fragmentée

Une annonce contient exactement trois trames avec le même numéro de
transaction :

| Opcode | `data[4]` | `data[5]` | `data[6]` | `data[7]` |
|---|---|---|---|---|
| `ANNOUNCE_BEGIN` | TYPE | ID courant, `0xFF` si non attribué | emplacement/port, `0xFF` si inconnu | état/flags d’identité |
| `ANNOUNCE_MAC_0` | MAC[0] | MAC[1] | MAC[2] | MAC[3] |
| `ANNOUNCE_MAC_1` | MAC[4] | MAC[5] | révision matérielle | réservé, zéro |

L’annonce est valide seulement si les trois trames du même numéro arrivent
dans les 500 ms, dans l’ordre, et que TYPE, ID et emplacement sont valides.
Les fragments isolés ou incomplets expirent sans modifier l’inventaire. La
MAC est l’identifiant matériel stable utilisé pour distinguer un remplacement
d’une carte déjà connue.

Les emplacements physiques sont codés ainsi : `1..8` = sortie physique 1..8,
`0` = carte racine/unique sans sortie amont (par exemple DISTRIB en mode sans
HUB), `0xFF` = emplacement non connu. La combinaison `slot=0xFF` n’est jamais
valide pour attribuer un ID D ou C.

### Registre des opcodes v1

| Opcode | Nom | Émetteur → destinataire | Fonction |
|---:|---|---|---|
| `0x01` | `HELLO` | S/H/D/C → tous | Présence périodique ; charge utile `{TYPE, ID, mode, topologie}`. Pour S, période 500 ms ; période H/D/C à fixer. |
| `0x02` | `ANNOUNCE_BEGIN` | H/D/C → S | Premier fragment d’identité ; suivi de `0x10` et `0x11`. |
| `0x03` | `DISCOVER_REQ` | S → H/D/C ciblé | Demande d’identification ; charge utile `{TYPE cible, ID cible, slot, flags}`. Exploration seulement. |
| `0x04` | Réservé | — | Une réponse positive à `DISCOVER_REQ` commence directement par `ANNOUNCE_BEGIN`. |
| `0x05` | `ASSIGN_BEGIN` | S → H/D/C ciblé | Attribution ; charge utile `{TYPE, ancien ID, slot, nouvel ID}`, suivie de fragments MAC. |
| `0x06` | `ASSIGN_ACK` | H/D/C → S | Confirmation après écriture ; charge utile `{TYPE, nouvel ID, slot, résultat}`. |
| `0x07` | `RUN_PERMIT` | S → H/D | Autorisation renouvelable ; charge utile `{TYPE source, ID source, topologie, TTL×10 ms}`. |
| `0x08` | `RUN_ACK` | H/D → S | Confirmation ou refus ; charge utile `{TYPE, ID, résultat, réserve}`. |
| `0x09` | `ERROR` | Tous → S | Erreur ; charge utile `{TYPE source, ID source, code, contexte}`. |
| `0x0A` | Réservé | — | `HELLO` est le message de présence canonique en v1. |
| `0x0B` | `DISCOVER_DONE` | S → tous | Fin de session de découverte validée ou annulée. |
| `0x0C` | `MODE_STATUS` | S → tous | Mode Exploration/Exploitation et topologie avec/sans HUB. |
| `0x10` | `ANNOUNCE_MAC_0` | H/D/C → S | Octets MAC 0..3 de l’annonce courante. |
| `0x11` | `ANNOUNCE_MAC_1` | H/D/C → S | Octets MAC 4..5, révision matérielle et réservé. |
| `0x12` | `ASSIGN_MAC_0` | S → H/D/C ciblé | Cible MAC, octets 0..3. |
| `0x13` | `ASSIGN_MAC_1` | S → H/D/C ciblé | Cible MAC, octets 4..5, flags d’écriture et réservé. |
| `0x80..0xFF` | Réservé | — | Réservé aux extensions futures. |

L’ancien opcode `0x40` de battement de bus est retiré en v1. Il ne vaut ni
`HELLO` ni `RUN_PERMIT` et n’est pas émis par les firmwares migrés. `HELLO`
est l’unique message de présence canonique.

### Charges utiles normatives des échanges

Les colonnes ci-dessous décrivent `data[4]` à `data[7]` :

| Message | Charge utile | Règle |
|---|---|---|
| `HELLO` | `{TYPE, ID, mode, topologie}` | Diffusé avec transaction 0. S émet toutes les 500 ms ; fréquence H/D/C à fixer. `mode`: 1 Exploration, 2 Exploitation, 3 bloqué. `topologie`: 1 avec HUB, 2 sans HUB. |
| `DISCOVER_REQ` | `{TYPE cible, ID cible, slot, flags}` | ID `0xFF` signifie « n’importe quelle ID de ce type » ; slot `0xFF` signifie inconnu/non applicable. La transaction doit être non nulle. |
| `ANNOUNCE_BEGIN` | `{TYPE, ID courant, slot, identityFlags}` | Réponse à la demande avec même transaction, puis les deux fragments MAC. `identityFlags`: bit 0 identité attribuée, bits 1..7 réservés à zéro. |
| `ASSIGN_BEGIN` | `{TYPE, ancien ID, slot, nouvel ID}` | Début d’écriture ciblée ; suivi de `ASSIGN_MAC_0/1` contenant la MAC cible exacte. |
| `ASSIGN_ACK` | `{TYPE, ID écrit, slot, résultat}` | `résultat=0` succès ; autre valeur = code d’erreur. Répond sur la transaction d’attribution. |
| `RUN_PERMIT` | `{TYPE source=S, ID source=1, topologie, TTL_10ms}` | Diffusé avec transaction non nulle toutes les 500 ms seulement si toutes les conditions d’Exploitation sont vraies. TTL vaut 150 (1500 ms). L’opcode implique le mode Exploitation. |
| `RUN_ACK` | `{TYPE H/D, ID, résultat, détail}` | Réponse positive ou refus ; `résultat=0` signifie autorisé, sinon code d’erreur. Répond sur le numéro de transaction du permis reçu. |
| `ERROR` | `{TYPE source, ID source, code, détail}` | Transaction copiée depuis le message fautif, ou 0 pour erreur spontanée. |
| `DISCOVER_DONE` | `{résultat, topologie, nombreValidé, réserve}` | résultat 0 validé, 1 annulé, 2 refusé. Les autres valeurs sont réservées à zéro. |
| `MODE_STATUS` | `{mode, topologie, statusFlags, réserve}` | Diffusé lors d’un changement de switch ou d’état ; les bits non définis valent zéro. |

Les fragments d’identité utilisent le même numéro de transaction et le même
type/ID/slot annoncés dans `ANNOUNCE_BEGIN`. Le fragment MAC et son en-tête
doivent arriver dans l’ordre `BEGIN`, `MAC_0`, `MAC_1`. Une attribution est
appliquée uniquement si la MAC complète correspond à celle ciblée par
l’inventaire S. Le nœud écrit d’abord son nouvel ID de façon persistante,
relit cette identité, puis envoie `ASSIGN_ACK`.

Chaque renouvellement `RUN_PERMIT` utilise un numéro de transaction non nul
distinct et prolonge le bail de 1500 ms. H/D répondent sur cette transaction.
H/D exigent à la fois un `HELLO` S frais et un `RUN_PERMIT` frais provenant de
`TYPE=S, ID=1`, avec une topologie conforme. L’expiration de l’un ou de
l’autre interdit le DCC. En Exploration, S émet `HELLO` avec mode 1 et
n’émet aucun `RUN_PERMIT`.

## 5. Format de télémétrie historiquement appelé `0x102`

Le numéro `0x102` désigne le format hérité de télémétrie, pas le CAN ID
physique des trames de supervision. Sur le CAN DCC local, le CAN ID réel reste
`0x102`. Sur le CAN de découverte, la trame utilise le CAN ID source unique
défini à la section 3.

### Trame locale B — CAN DCC du segment

| Octet | Champ | Valeur |
|---:|---|---|
| 0 | ID canton | 1..252 attribué par CC/EXCC |
| 1 | TYPE | 5 (B) dans le nouveau protocole |
| 2 | courant | courant en unités de 10 mA |
| 3 | tension | tension en unités de 100 mV |
| 4 | état | `0=OK`, `1=FAULT`, `2=OVERHEAT`, `3=OFF` |

DLC 5. Cette trame locale ne change pas lorsque le convertisseur la relaie.

### Copie B relayée — CAN de découverte

| Octet | Champ | Valeur |
|---:|---|---|
| 0..4 | Champs B d’origine | Copie exacte de la trame locale |
| 5 | ID source C | Identité du convertisseur ayant relayé la trame |

DLC 6 sur le CAN de découverte. TYPE reste B (5). Le convertisseur C ne
modifie ni l’ID de canton ni les mesures. Le CAN ID d’arbitrage et `data[5]`
portent tous deux l’ID source C ; la carte S vérifie leur cohérence et associe
C à son emplacement dans l’inventaire. Si C n’a pas d’ID C valide, il ne
relaie pas cette télémétrie.

La trame relayée n’est jamais retransmise du bus de découverte vers le CAN DCC.
HUB et DISTRIB peuvent lire les cinq premiers octets et ignorer `data[5]`.

### Télémétrie C et D

Format commun de base, DLC 5 :
`{ ID, TYPE, mesure1, mesure2, état }`. Pour C et D sans mesure électrique,
les octets de mesure valent zéro. Le mapping exact de `state` pour D/C et les
messages de présence sont à fixer pour la version 1.

## 6. Découverte et attribution

1. La carte S ne lance une découverte qu’en mode Exploration et après avoir
   validé la microSD, le JSON et la présence de l’ERM sur le bus.
2. S demande l’inhibition DCC au contrôleur de la topologie sélectionnée : H
   en mode avec HUB, D racine en mode sans HUB.
3. Le nœud ciblé répond avec `ANNOUNCE_BEGIN`, puis les deux fragments MAC.
   Les trois trames reprennent le numéro de transaction de la requête, afin de
   distinguer les réponses anciennes des réponses courantes.
4. S vérifie les MAC/IDs connus, les doublons et l’emplacement. Une association
   physique incertaine n’est jamais attribuée.
5. S envoie `ASSIGN_BEGIN` puis `ASSIGN_MAC_0/1`; le nœud écrit l’identité dans
   son stockage et retourne `ASSIGN_ACK` avec les valeurs réellement
   enregistrées.
6. S ne valide l’emplacement qu’après ACK correspondant à la session, au type,
   à la MAC et à l’ID demandé.
7. S valide l’ensemble de l’inventaire, écrit le JSON et vérifie SD et mémoire
   interne avant de pouvoir autoriser l’exploitation.

Le convertisseur peut demander sa découverte par le bouton physique uniquement
en mode Exploration. Le mécanisme TOR pour associer cette demande à une sortie
D reste à définir ; tant qu’il n’est pas spécifié, aucun ID C ne peut être
attribué à partir d’une annonce CAN seule.

La carte B ne participe pas à ce cycle et son ID n’est jamais attribué par S.

## 7. Autorisation d’exploitation et présence S

`RUN_PERMIT` est une autorisation à durée limitée, renouvelée par S toutes les
500 ms avec un TTL de 1 500 ms. Elle est acceptée uniquement par H ou D après
validation de l’état local. L’ancien battement `0x40` ne peut jamais libérer
une sortie et n’est pas utilisé en v1.

- Au démarrage, H et D maintiennent leurs sorties DCC bloquées jusqu’à une
  autorisation valide.
- Un `HELLO` S annonçant Exploration ou bloqué provoque immédiatement
  l’inhibition ; S n’émet alors aucune autorisation RUN.
- En Exploitation, S renouvelle l’autorisation tant que ses vérifications
  restent valides.
- En absence ou invalidité de S pendant le délai configuré, H/D coupent leurs
  sorties DCC et indiquent la cause.
- Au retour de S, H/D restent coupés jusqu’à une nouvelle séquence valide.
- Une topologie incompatible (switch sans HUB alors que H répond, ou switch
  avec HUB alors que le H obligatoire ne répond pas) est une erreur bloquante,
  jamais une bascule automatique.

Les valeurs de `RUN_PERMIT` sont retenues : renouvellement toutes les 500 ms,
TTL de 1 500 ms et coupure après 1 500 ms sans `HELLO` S valide ou sans permis
valide. Le temps physique entre l’expiration du permis et le blocage effectif
des sorties doit être mesuré et validé sur le matériel.

## 8. Temporisations

| Paramètre | Valeur de travail | Statut |
|---|---:|---|
| Présence `HELLO` de S | 500 ms | Validé |
| Expiration de S côté H/D et coupure DCC | 1 500 ms sans `HELLO` valide de S ou `RUN_PERMIT` | Validé |
| Réponse initiale à une requête de découverte | 500 ms | Validé |
| Alerte de transaction sans réponse | 500 ms pour `DISCOVER_REQ` | Validé |
| Intervention requise pour carte absente | 600 000 ms (10 min) | Décision retenue, configurable |
| Renouvellement `RUN_PERMIT` | Toutes les 500 ms ; TTL 1 500 ms | Retenu avec les délais de présence S |
| Retransmission d’attribution | 250 ms, jusqu’à 3 envois | Proposition ; à valider |
| Délai de complétion de fragments d’annonce | 500 ms | Proposition ; à valider |
| Découverte séquentielle d’un emplacement | 1 500 ms | Héritage du firmware existant ; à confirmer |

Les temporisations de dialogue utilisent des délais monotones. Les comparaisons
de millis doivent rester sûres au débordement. Une transaction expirée produit
une erreur explicite et n’autorise pas l’exploitation.

## 9. Erreurs et réponses

Registre de codes proposé pour `ERROR` et `RUN_ACK` négatif :

| Code | Nom | Réaction S | Réaction H/D/C |
|---:|---|---|---|
| `0x01` | `BAD_FRAME` | Journaliser et afficher erreur protocole | Ignorer la trame invalide |
| `0x02` | `UNSUPPORTED_VERSION` | Bloquer la découverte/exploitation | Refuser la transaction, rester DCC coupé |
| `0x03` | `WRONG_MODE` | Signaler une demande interdite en exploitation | Ne pas lancer la découverte |
| `0x04` | `DUPLICATE_ID` | Bloquer validation et demander intervention | Ne pas écraser l’ID courant |
| `0x05` | `MAC_CONFLICT` | Bloquer et demander vérification manuelle | Ne pas écraser l’identité |
| `0x06` | `WRONG_SLOT` | Bloquer l’association | Ne pas appliquer l’ID |
| `0x07` | `UNKNOWN_NODE` | Marquer carte inconnue | Rester inhibée jusqu’à validation |
| `0x08` | `NO_RESPONSE` | Alerte ; démarrer le délai d’absence | Maintenir DCC interdit si la cible est H/D |
| `0x09` | `ASSIGN_FAILED` | Ne pas valider la mise à jour | Garder ancienne identité ou rester non attribuée |
| `0x0A` | `STORAGE_NOT_READY` | Rester en Exploration | Refuser RUN_PERMIT |
| `0x0B` | `TOPOLOGY_MISMATCH` | Afficher conflit avec switch | Garder les sorties DCC coupées |
| `0x0C` | `SOURCE_ID_INVALID` | Rejeter télémétrie relayée | C ne relaie pas avec source ambiguë |
| `0x0D` | `INVENTORY_MISMATCH` | Rester en Exploration | Refuser exploitation |
| `0x0E` | `RUN_DENIED` | Afficher le motif de refus | Maintenir sorties coupées |
| `0x0F` | `SESSION_MISMATCH` | Ignorer réponse ancienne/hors session | Ne pas valider une réponse non corrélée |

Les détails de codage du champ code, du nœud source, de la session et du texte
de diagnostic sont à figer dans la version 1. Les erreurs ne doivent jamais
entraîner une libération implicite du DCC.

## 10. Versionnement et compatibilité

- La version du protocole sera explicite dans le format de contrôle final.
- Un nœud ne doit pas accepter un `RUN_PERMIT` d’une version qu’il ne comprend
  pas.
- Les changements des types (`S=1, H=2, D=3, C=4, B=5`), des CAN ID
  d’arbitrage de supervision et du relais B DLC 6 sont incompatibles avec les
  firmwares actuels qui utilisent d’autres registres et filtres.
- Toute migration sera faite de façon coordonnée sur les firmwares S/H/D/C/B
  concernés, mais le firmware B conservera son attribution de canton CC.
- Les versions de la bibliothèque partagée seront figées par dépendance dans
  `platformio.ini`, sans téléchargement automatique d’une version flottante.

## 11. Points à trancher avant de publier v1

1. Fréquence `HELLO` de H/D/C hors S.
2. Confirmer que le CAN ID source C et `data[5]` C suffisent pour situer un
   relais B dans toutes les topologies.
3. Nombre de retransmissions d’attribution.
4. Codes d’état des cartes S/H/D/C.
5. Transport fiable et sûr de l’inhibition et de l’autorisation RUN.
6. Réaction détaillée à la perte de S puis à son retour.
7. Cas où une sortie est volontairement non équipée et validation humaine
   d’une MAC de remplacement.
8. Détails des flags et de la révision matérielle dans les annonces.

La version 1 ne doit être déclarée stable qu’après validation de ces points et
publication de trames de référence testables par chaque firmware.
