#pragma once

#include <stdint.h>

// Protocole logique DCC partagé par les firmwares CFC.
// Les fonctions d'encodage/décodage restent dans les firmwares; ce fichier
// fixe les identifiants, les valeurs et les dispositions de données.
namespace cfc {
namespace dcc {

static constexpr uint8_t PROTOCOL_VERSION = 1;

// Identifiants source du CAN Service 29 bits. Ils sont distincts des
// identités de découverte 11 bits (par exemple LaBox L/1 et ERM S/1).
// Les CC utilisent le numéro de canton comme source CAN Service.
static constexpr uint16_t SERVICE_NODE_ID_CC_MIN = 1;
static constexpr uint16_t SERVICE_NODE_ID_CC_MAX = 250;
static constexpr uint16_t SERVICE_NODE_ID_NEW_CARD = 251;
static constexpr uint16_t SERVICE_NODE_ID_CLF = 252;
static constexpr uint16_t SERVICE_NODE_ID_ERM_LIAISON_CFC = 253;
static constexpr uint16_t SERVICE_NODE_ID_LABOX = 254;
static constexpr uint16_t SERVICE_NODE_ID_RESERVED = 255;
static constexpr uint16_t SERVICE_NODE_ID_CONVERTER_BASE = 0x0100;

// Pour un convertisseur dont l'identité de découverte est C/11..C/88,
// la source CAN Service vaut 0x0100 + l'ID de découverte.
constexpr uint16_t converterServiceNodeId(uint8_t discoveryId) {
    return SERVICE_NODE_ID_CONVERTER_BASE + discoveryId;
}

// CAN Booster : CAN standard 11 bits. 0x100 transporte le flux DCC/cutout;
// 0x101 sert uniquement à annoncer l'ID Booster en Exploration autorisée.
static constexpr uint16_t CAN_BOOSTER_STREAM_ID = 0x100;
static constexpr uint16_t CAN_BOOSTER_DISCOVERY_ID = 0x101;
static constexpr uint8_t BOOSTER_DISCOVERY_FORMAT_VERSION = 1;
static constexpr uint8_t BOOSTER_DISCOVERY_DLC = 8;
static constexpr uint8_t STREAM_MAGIC = 0xD2;
static constexpr uint8_t STREAM_DLC_DCC_1_TO_8_BITS = 5;
static constexpr uint8_t STREAM_DLC_DCC_9_TO_16_BITS = 6;
static constexpr uint8_t STREAM_DLC_CUTOUT = 7;
static constexpr uint8_t MAX_DCC_BITS_PER_BLOCK = 16;
static constexpr uint16_t BOOSTER_NO_BLOCK_FAILSAFE_MS = 50;

enum class StreamRecordType : uint8_t {
    DCC_BITS = 0,
    CUTOUT = 1
};

// Octets du message CAN Booster 0x100 :
// [0] 0xD2; [1..2] séquence uint16 LE; [3] contrôle;
// [4..5] bits DCC empaquetés MSB-first, ou durée cutout uint16 LE en µs;
// [6] drapeaux cutout (bit 0 local, bit 1 global; autres bits à zéro).
// Contrôle [3] : bits 7..6 type; bit 5 début de paquet DCC;
// bits 4..0 nombre de bits (1..16 pour DCC, exactement 0 pour cutout).
static constexpr uint8_t STREAM_CONTROL_TYPE_MASK = 0xC0;
static constexpr uint8_t STREAM_CONTROL_PACKET_START = 0x20;
static constexpr uint8_t STREAM_CONTROL_COUNT_MASK = 0x1F;
static constexpr uint8_t STREAM_CUTOUT_FLAG_LOCAL = 0x01;
static constexpr uint8_t STREAM_CUTOUT_FLAG_GLOBAL = 0x02;

// CAN Service : CAN étendu 29 bits, convention CanUniversal/Locoduino.
// Le champ commande contient l'opcode; le champ réponse vaut 0 pour une
// demande/annonce et 1 pour une réponse. Le nodeId de l'identifiant reste la
// source du message. Les champs cible sont donc dans la charge utile si requis.
enum class ServiceCommand : uint8_t {
    BOOSTER_SUPERVISION = 0xB6,
    LABOX_DCC_PROFILE = 0xB7,
    LABOX_CUTOUT_REFERENCE = 0xB8,
    CONVERTER_DCC_FAULT = 0xB9,
    BOOSTER_TIMING_SET = 0xBA,
    BOOSTER_DETECTED = 0xBB,
    BOOSTER_ASSOCIATION_VALIDATION = 0xC4,
    BOOSTER_DISCOVERY_MODE = 0xC5,
    BOOSTER_TIMING_READ = 0xC6,
    BOOSTER_RAIL_VOLTAGE = 0xC7,
    LABOX_VOLTAGE_TELEMETRY = 0xC8,
    LABOX_PROFILE_SETPOINT = 0xC9
};

static constexpr uint8_t PROFILE_SETPOINT_TARGET_LABOX = 1;
static constexpr uint8_t PROFILE_SETPOINT_TARGET_ALL_CC = 2;
static constexpr uint8_t TRACK_PROFILE_N = 0;
static constexpr uint8_t TRACK_PROFILE_HO = 1;

static constexpr uint8_t SERVICE_DLC = 8;
static constexpr uint8_t BOOSTER_RAIL_VOLTAGE_DLC = 8;
static constexpr uint8_t BOOSTER_RAIL_VOLTAGE_VALID = 1;
static constexpr uint8_t BOOSTER_RAIL_VOLTAGE_INVALID = 0;
static constexpr uint8_t LABOX_VOLTAGE_TELEMETRY_DLC = 8;
static constexpr uint16_t LABOX_VOLTAGE_TELEMETRY_PERIOD_MS = 1000;
static constexpr uint16_t LABOX_VOLTAGE_TELEMETRY_STALE_MS = 3000;
static constexpr uint16_t VOLTAGE_MEASUREMENT_INVALID_MV = 0xFFFF;
static constexpr uint16_t REFERENCE_PERIOD_MS = 500;
static constexpr uint16_t REFERENCE_STALE_MS = 1500;
static constexpr uint16_t SUPERVISION_PERIOD_MS = 1000;
static constexpr uint16_t SUPERVISION_STALE_MS = 3000;
static constexpr uint16_t BOOSTER_RAIL_VOLTAGE_PERIOD_MS = 1000;
static constexpr uint16_t BOOSTER_RAIL_VOLTAGE_STALE_MS = 3000;
static constexpr uint16_t CONVERTER_FAULT_REPEAT_MS = 1000;
static constexpr uint16_t BOOSTER_ANNOUNCE_PERIOD_MS = 100;
static constexpr uint16_t BOOSTER_DETECTION_REPEAT_MS = 1000;
static constexpr uint16_t BOOSTER_DISCOVERY_MODE_PERIOD_MS = 500;
static constexpr uint16_t BOOSTER_DISCOVERY_MODE_TTL_MS = 1500;
static constexpr uint16_t BOOSTER_ASSOCIATION_RETRY_MS = 500;
static constexpr uint16_t BOOSTER_ASSOCIATION_UART_TIMEOUT_MS = 1500;

enum class BoosterState : uint8_t {
    OK = 0,
    FAULT = 1,
    OVERHEAT = 2,
    OFF = 3
};

enum class BoosterDccCause : uint8_t {
    NONE = 0,
    SEQUENCE_GAP = 1,
    STREAM_TIMEOUT = 2,
    MALFORMED_STREAM_FRAME = 3,
    PROFILE_MISSING = 4,
    PROFILE_INVALID = 5,
    PROFILE_STALE = 6,
    GENERATOR_ERROR = 7
};

// BOOSTER_SUPERVISION (0xB6), DLC 8, source CC (CAN Service ID equals its
// canton ID, allocated 1..250) -> ERM and converter:
// [0] supply voltage / 100 mV; [1] current / 10 mA;
// [2] BoosterState; [3] BoosterDccCause (0 means no active DCC cause);
// [4..5] expected stream sequence uint16 LE;
// [6..7] received stream sequence uint16 LE.
// Sequence fields are zero unless cause == SEQUENCE_GAP. Emit on change and
// at least once per second. Consumer marks stale at 3000 ms.

// LABOX_DCC_PROFILE (0xB7), DLC 8, LaBox CAN Service source ID 254.
// Son identité de découverte reste L/1.
// [0] format version; [1..2] reference sequence uint16 LE;
// [3] flags: bit 0 VALID, bit 1 MAIN_ON, all other bits zero;
// [4] half-bit 1 µs; [5] half-bit 0 µs; [6] timer tick µs;
// [7] MAIN preamble bit count. Emit every 500 ms. The CC forwards the same
// payload to EXCC; UART DCC_REFERENCE (0x06) carries this same 8-byte payload.
static constexpr uint8_t PROFILE_FLAG_VALID = 0x01;
static constexpr uint8_t PROFILE_FLAG_MAIN_ON = 0x02;

// LABOX_CUTOUT_REFERENCE (0xB8), DLC 8, LaBox CAN Service source ID 254.
// [0] format version; [1..2] same reference sequence as 0xB7, uint16 LE;
// [3] flags: bit 0 measured reference VALID, bit 1 RailCom enabled,
// bit 2 RailCom paused; all other bits zero;
// [4..5] measured effective cutout duration in µs, uint16 LE;
// [6..7] reserved, zero. If VALID is clear, duration must be zero.
// Measurement value remains unavailable until bench measurement at HUB output.
static constexpr uint8_t CUTOUT_FLAG_VALID = 0x01;
static constexpr uint8_t CUTOUT_FLAG_RAILCOM_ENABLED = 0x02;
static constexpr uint8_t CUTOUT_FLAG_RAILCOM_PAUSED = 0x04;

enum class ConverterFaultCause : uint8_t {
    HALF_BIT_OUT_OF_TOLERANCE = 0x01,
    CUTOUT_OUT_OF_TOLERANCE = 0x02,
    LABOX_REFERENCE_MISSING_OR_STALE = 0x03,
    DCC_STREAM_INCONSISTENT_OR_SEND_FAILED = 0x04
};

// CONVERTER_DCC_FAULT (0xB9), DLC 8, converter source ID
// 0x0100 + its discovery ID (C/11..C/88 maps to 0x010B..0x0158):
// [0] district 1..8; [1] ConverterFaultCause; [2] 1 active / 0 resolved;
// [3..7] reserved, zero. Emit on state change and repeat active causes each s.

// BOOSTER_TIMING_SET (0xBA), DLC 8. Request (resp=0), source ERM Liaison CFC
// ID 253:
// [0] format version; [1..2] transaction ID uint16 LE;
// [3] target canton/booster ID (1..250 in the next protocol revision; current
// published firmware still accepts 1..252);
// [4] signed half-bit-1 correction µs;
// [5] signed half-bit-0 correction µs; [6..7] reserved, zero.
// Each correction must be -5..+5. Response (resp=1) uses same opcode:
// [0] format version; [1..2] same transaction ID; [3] TimingResult;
// [4] saved half-bit-1 correction; [5] saved half-bit-0 correction;
// [6..7] reserved, zero. On refusal, bytes [4..5] report the old saved values.
// Booster rejects requests unless DCC output is stopped. A request is complete
// only after local persistence succeeds; the response travels back via CC.
enum class TimingResult : uint8_t {
    APPLIED = 0,
    OUTPUT_ACTIVE = 1,
    CORRECTION_OUT_OF_RANGE = 2,
    RESULTING_TIMING_INVALID = 3,
    STORAGE_FAILED = 4,
    BOOSTER_UNAVAILABLE = 5,
    TRANSACTION_CONFLICT = 6
};

// BOOSTER_TIMING_READ (0xC6), CAN Service 29-bit, DLC 8.
// Request (resp=0), ERM Liaison CFC source ID 253:
// [0] format version; [1..2] transaction uint16 LE; [3] target booster ID;
// [4..7] reserved, zero. Response (resp=1), CC source, same target in nodeId:
// [0] format version; [1..2] transaction; [3] TimingResult (APPLIED means the
// read succeeded); [4..5] saved signed half-bit corrections in µs;
// [6..7] reserved, zero. This is read-only and may be requested while DCC runs.

// BOOSTER_RAIL_VOLTAGE (0xC7), CAN Service 29-bit, DLC 8, CC -> ERM Liaison
// CFC. The CC source/node ID (1..250) identifies the reporting canton; the Booster ID
// is known from its CC/EXCC association. Payload: [0..1] measured rail voltage
// in millivolts, uint16 LE; [2] validity (1 valid, 0 invalid); [3..7] reserved,
// zero. The useful three measurement bytes are copied unchanged from CAN
// Commands BOOSTER_RAIL_VOLTAGE (0xDD). The CC emits every 1000 ms; the ERM
// marks this measurement stale after 3000 ms without a valid update.

// LABOX_VOLTAGE_TELEMETRY (0xC8), CAN Service 29-bit, DLC 8, LaBox source ID
// 254 (discovery identity remains L/1). Four uint16 LE readings in millivolts: [0..1] general supply,
// [2..3] MP1584 5 V output, [4..5] XL4015 output, [6..7] PROG track voltage
// after DRV8874. A reading of 0xFFFF means that measurement is invalid or
// unavailable. LaBox emits every 1000 ms; the ERM marks the telemetry stale
// after 3000 ms without a new valid frame. 0xC9 is reserved for the profile /
// setpoint command; the next local draft defines its request and response.
// That draft is not yet published. CAN Booster carries no copy of telemetry.

// LABOX_PROFILE_SETPOINT (0xC9), CAN Service 29-bit, DLC 8. Request (resp=0),
// source ERM_Liaison_CFC ID 253:
// [0] format version (=1); [1] target (1=LaBox, 2=all CC);
// [2] profile (0=N, 1=HO); [3..4] target rail voltage in mV, uint16 LE;
// [5..6] transaction ID, uint16 LE; [7] reserved zero.
// Accepted only during Exploration. A rejected request changes no state.
// Reject target/profile values outside the enumerated values, zero/0xFFFF
// voltage, and a voltage outside the shared configured limits. Those numeric
// limits are intentionally supplied by configuration and are not fixed here.
// Each responding card uses the same opcode with resp=1 and its own source
// ID (LaBox 254, CC canton ID 1..250). The response echoes bytes [0..6] and
// places ProfileSetpointResult in byte [7]. A CC responds only after Booster
// application confirmation. This confirms acceptance, not that measured rail
// voltage has reached the setpoint; 0xC7/0xC8 telemetry verifies that later.
// This command does not start PROG, HUB or a Booster output. For an all-CC
// request, every participating CC must answer; a missing answer leaves the
// configuration incomplete and outputs stopped. The ERM makes three total
// sends 500 ms apart (1500 ms overall). Each recipient keeps the latest
// transaction ID, exact request and result in volatile memory. An identical
// retry returns the saved result; changed data with that ID returns result 7
// with no state change. A new transaction replaces the record; reboot clears
// it. The ERM completes or times out a transaction before starting another.
enum class ProfileSetpointResult : uint8_t {
    ACCEPTED = 0,
    NOT_IN_EXPLORATION = 1,
    INVALID_RECIPIENT = 2,
    INVALID_PROFILE = 3,
    INVALID_SETPOINT = 4,
    BOOSTER_UNCONFIRMED = 5,
    APPLICATION_FAILED = 6,
    TRANSACTION_CONFLICT = 7
};

// Response (resp=1), DLC 8, source is the responding LaBox/CC:
// [0] version (=1); [1] recipient class echoed (1=LaBox, 2=CC);
// [2] profile echoed (0=N, 1=HO); [3..4] setpoint mV uint16 LE echoed;
// [5..6] transaction uint16 LE echoed; [7] ProfileSetpointResult.
// ERM validates the recipient selector before transmission. Nodes ignore an
// unknown selector without replying. INVALID_RECIPIENT is defensive only when
// a uniquely dispatched handler detects a recipient mismatch.
// Numeric profile voltage limits are configured in the shared configuration.
// Unique source IDs distinguish same-opcode responses; no 0xCA/0xCB response
// opcodes are used. EXCC retries UART 0x1A once after 500 ms; if no application
// response arrives within 1000 ms of the first attempt, it returns result 5.
// UART result 5 maps to CAN result 7 through the EXCC/CC relay. Each recipient
// keeps only its latest transaction record in volatile memory; a new
// transaction replaces it and reboot clears it. The ERM does not overlap
// transactions.
// Local draft relay path: CC -> EXCC CAN Commands 0xE6 (same 8 request bytes),
// EXCC -> Booster UART 0x1A (same payload), Booster -> EXCC UART 0x1B
// (transaction uint16 LE + application result), EXCC -> CC CAN Commands 0xDE
// (transaction uint16 LE + ProfileSetpointResult + 5 zero bytes). UART result
// values map as UART 0->CAN 0, 1->1, 2->3, 3->4, 4->6, 5->7. Application
// timeout uses CAN result 5 after the defined retry/deadline.

enum class BoosterAssociationResult : uint8_t {
    VALID = 0,
    ID_MISMATCH = 1,
    UNAVAILABLE = 2,
    BOOSTER_REFUSED = 3
};

// Mapping for the existing UART ACK status when acknowledging UART 0x16:
// 0 (ACK_OK)->VALID; 1 (ACK_INVALID)->ID_MISMATCH; 2 (ACK_NO_HEARTBEAT) or
// no ACK before timeout -> UNAVAILABLE; 3 (ACK_NO_ID)->BOOSTER_REFUSED.

// CAN Booster 0x101, standard 11-bit, DLC 8, response to fresh Exploration
// permit only:
// [0] format version (=1); [1] saved canton/booster ID (1..250 in the next
// protocol revision; current firmware still accepts 1..252);
// [2..7] reserved zero. Booster emits every 100 ms only while it has a valid
// runtime ID and a fresh Exploration permit. A changed ID clears the local
// association latch and restarts announcements; same-ID refresh is idempotent.
// It stops after association is validated. CAN DCC/cutout data remains 0x100.

// BOOSTER_DETECTED (0xBB), CAN Service 29-bit, DLC 8, converter source ID
// 0x0100 + its discovery ID (C/11..C/88 maps to 0x010B..0x0158):
// [0] format version (=1); [1] booster ID (1..250 in the next protocol
// revision; current firmware still accepts 1..252); [2..7] reserved zero;
// resp=0. Converter repeats every second until it hears a successful 0xC4
// response from the CC for this booster ID.

// BOOSTER_ASSOCIATION_VALIDATION (0xC4), CAN Service 29-bit, DLC 8.
// Request (resp=0), ERM Liaison CFC source ID 253:
// [0] format version (=1); [1..2] transaction uint16 LE;
// [3] target booster/canton ID; [4] zero; [5..7] reserved zero.
// Only the CC whose canton ID matches [3] handles the broadcast; other CCs
// ignore it without replying. That CC relays the unchanged payload to EXCC
// (CAN Command 0xEE), then to the booster (UART 0x16). Response (resp=1),
// CC source:
// [0..3] copied; [4] BoosterAssociationResult; [5..7] reserved zero. This
// response is visible to the converter on CAN Service, which then stops 0xBB.
// Results: 0 accepted, 1 ID mismatch, 2 EXCC/booster unavailable or UART
// timeout (1500 ms), 3 booster refused. ERM retries every 500 ms with the same
// transaction until a result; CC/EXCC deduplicate pending requests.

// BOOSTER_DISCOVERY_MODE (0xC5), CAN Service 29-bit, DLC 8, ERM Liaison CFC
// source ID 253:
// [0] format version (=1); [1] 1=permit / 0=inhibit; [2..7] reserved zero.
// Repeat every 500 ms. A permit expires after 1500 ms without refresh.
// CC relays it as CAN Command 0xEF and EXCC sends UART 0x15 to its booster.

// Types within the existing UART frame (frame v2, CRC-16, transport sequence
// and ACK type 0x04 remain unchanged).
enum class UartType : uint8_t {
    DCC_REFERENCE = 0x06,
    DCC_DIAGNOSTIC = 0x12,
    DCC_TIMING_SET = 0x13,
    DCC_TIMING_RESULT = 0x14,
    BOOSTER_DISCOVERY_MODE = 0x15,
    BOOSTER_ASSOCIATION_VALIDATE = 0x16,
    CLEAR_CANTON_ID = 0x17,
    DCC_TIMING_READ = 0x18
};

// UART BOOSTER_DISCOVERY_MODE (0x15), payload DLC 1: [0] 1=permit / 0=inhibit.
// Permit expires after 1500 ms without refresh; inhibition is immediate.
// UART BOOSTER_ASSOCIATION_VALIDATE (0x16), payload DLC 8: unchanged Service
// 0xC4 request payload. Existing UART ACK 0x04, correlated by command and
// sequence, carries the acceptance status; EXCC translates it to CAN Command
// result 0xD3 through CC. No extra UART result frame is introduced.
// UART CLEAR_CANTON_ID (0x17), payload DLC 0: explicit request to erase the
// persisted booster ID and stop DCC. Ordinary SET_CANTON_ID with 0xFF is not
// an erase request: it marks the runtime ID unassigned and inhibits DCC while
// preserving the persisted ID. All three commands require a UART ACK.

// UART DCC_DIAGNOSTIC (0x12), payload DLC 8:
// [0] canton ID; [1] BoosterDccCause; [2] 1 active / 0 cleared;
// [3] bit 0 means sequence fields are present; other bits zero;
// [4..5] expected sequence uint16 LE; [6..7] received sequence uint16 LE.
// Transport ACK uses the UART frame sequence; retransmit with the same
// transport sequence if its ACK is missing. First cause stays latched to safe
// recovery; clear report uses the same cause and active=0.

// UART DCC_TIMING_SET (0x13), payload DLC 4:
// [0..1] transaction ID copied from CAN request, uint16 LE;
// [2] signed half-bit-1 correction µs; [3] signed half-bit-0 correction µs.
// UART DCC_TIMING_RESULT (0x14), payload DLC 5:
// [0..1] transaction ID; [2] TimingResult; [3] saved half-bit-1 correction;
// [4] saved half-bit-0 correction. UART transport ACK and application result
// are distinct: ACK confirms frame reception; RESULT confirms applied/saved.
// UART DCC_TIMING_READ (0x18), payload DLC 2: [0..1] transaction uint16 LE.
// The Booster answers with DCC_TIMING_RESULT (0x14), reporting its stored
// corrections without changing them; the ordinary UART ACK confirms receipt.

} // namespace dcc
} // namespace cfc
