#pragma once

#include <stdint.h>

// Protocole logique DCC partagé par les firmwares CFC.
// Les fonctions d'encodage/décodage restent dans les firmwares; ce fichier
// fixe les identifiants, les valeurs et les dispositions de données.
namespace cfc {
namespace dcc {

static constexpr uint8_t PROTOCOL_VERSION = 1;

// CAN Booster : CAN standard 11 bits, flux DCC/cutout seulement.
static constexpr uint16_t CAN_BOOSTER_STREAM_ID = 0x100;
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
    BOOSTER_TIMING_SET = 0xBA
};

static constexpr uint8_t SERVICE_DLC = 8;
static constexpr uint16_t REFERENCE_PERIOD_MS = 500;
static constexpr uint16_t REFERENCE_STALE_MS = 1500;
static constexpr uint16_t SUPERVISION_PERIOD_MS = 1000;
static constexpr uint16_t SUPERVISION_STALE_MS = 3000;
static constexpr uint16_t CONVERTER_FAULT_REPEAT_MS = 1000;

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

// BOOSTER_SUPERVISION (0xB6), DLC 8, CC -> ERM and converter:
// [0] supply voltage / 100 mV; [1] current / 10 mA;
// [2] BoosterState; [3] BoosterDccCause (0 means no active DCC cause);
// [4..5] expected stream sequence uint16 LE;
// [6..7] received stream sequence uint16 LE.
// Sequence fields are zero unless cause == SEQUENCE_GAP. Emit on change and
// at least once per second. Consumer marks stale at 3000 ms.

// LABOX_DCC_PROFILE (0xB7), DLC 8, LaBox source ID 1:
// [0] format version; [1..2] reference sequence uint16 LE;
// [3] flags: bit 0 VALID, bit 1 MAIN_ON, all other bits zero;
// [4] half-bit 1 µs; [5] half-bit 0 µs; [6] timer tick µs;
// [7] MAIN preamble bit count. Emit every 500 ms. The CC forwards the same
// payload to EXCC; UART DCC_REFERENCE (0x06) carries this same 8-byte payload.
static constexpr uint8_t PROFILE_FLAG_VALID = 0x01;
static constexpr uint8_t PROFILE_FLAG_MAIN_ON = 0x02;

// LABOX_CUTOUT_REFERENCE (0xB8), DLC 8, LaBox source ID 1:
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

// CONVERTER_DCC_FAULT (0xB9), DLC 8, converter source identity:
// [0] district 1..8; [1] ConverterFaultCause; [2] 1 active / 0 resolved;
// [3..7] reserved, zero. Emit on state change and repeat active causes each s.

// BOOSTER_TIMING_SET (0xBA), DLC 8. Request (resp=0), source ERM ID 1:
// [0] format version; [1..2] transaction ID uint16 LE;
// [3] target canton/booster ID 1..252; [4] signed half-bit-1 correction µs;
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

// Types within the existing UART frame (frame v2, CRC-16, transport sequence
// and ACK type 0x04 remain unchanged).
enum class UartType : uint8_t {
    DCC_REFERENCE = 0x06,
    DCC_DIAGNOSTIC = 0x12,
    DCC_TIMING_SET = 0x13,
    DCC_TIMING_RESULT = 0x14
};

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

} // namespace dcc
} // namespace cfc
