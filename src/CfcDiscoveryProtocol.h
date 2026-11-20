#pragma once

#include <stddef.h>
#include <stdint.h>

// Valeurs partagées du CAN de découverte CFC, version 0.3.0.
// Les CAN ID des segments DCC locaux ne sont pas définis par cette bibliothèque.
namespace cfc {
namespace discovery {

static constexpr uint8_t PROTOCOL_VERSION_MAJOR = 1;
static constexpr uint16_t CAN_ID_BASE = 0x500;
static constexpr uint16_t CAN_ID_CLASS_STRIDE = 0x80;
static constexpr uint16_t CAN_ID_MAX_STANDARD = 0x7FF;
static constexpr uint8_t ID_UNASSIGNED = 0x7F;
static constexpr uint8_t ID_UNKNOWN = 0xFF;
static constexpr uint8_t SLOT_ROOT = 0;
static constexpr uint8_t SLOT_UNKNOWN = 0xFF;
static constexpr uint8_t CONTROL_DLC = 8;
static constexpr uint8_t TELEMETRY_B_LOCAL_DLC = 5;
static constexpr uint8_t TELEMETRY_B_RELAY_DLC = 6;
static constexpr uint8_t LABOX_REFERENCE_DLC = 8;
static constexpr uint32_t LABOX_REFERENCE_PERIOD_MS = 500;

static constexpr uint32_t S_HELLO_PERIOD_MS = 500;
static constexpr uint32_t RUN_PERMIT_PERIOD_MS = 500;
static constexpr uint32_t RUN_PERMIT_TTL_MS = 1500;
static constexpr uint32_t NODE_LOSS_TIMEOUT_MS = 1500;
static constexpr uint32_t DISCOVERY_RESPONSE_TIMEOUT_MS = 500;
static constexpr uint32_t ANNOUNCE_FRAGMENT_TIMEOUT_MS = 500;
static constexpr uint32_t ABSENCE_INTERVENTION_MS = 600000;
static constexpr uint32_t NODE_HELLO_PERIOD_MS = 500;
static constexpr uint8_t ASSIGN_MAX_ATTEMPTS = 3;
static constexpr uint32_t ASSIGN_RETRY_PERIOD_MS = 250;

// Repérage physique par les lignes TOR dédiées, uniquement en Exploration.
// D -> H : préambule HIGH 100 ms, LOW 100 ms ; puis 56 symboles MAC+CRC8.
// Chaque symbole est HIGH 20 ms (0) ou 40 ms (1), puis LOW 20 ms.
static constexpr uint16_t D_LOCATOR_PREAMBLE_MS = 100;
static constexpr uint16_t D_LOCATOR_ZERO_HIGH_MS = 20;
static constexpr uint16_t D_LOCATOR_ONE_HIGH_MS = 40;
static constexpr uint16_t D_LOCATOR_LOW_GAP_MS = 20;
static constexpr uint8_t D_LOCATOR_BITS = 56;
static constexpr uint32_t D_LOCATOR_REPEAT_MS = 10000;
// C -> D : trois impulsions de 100 ms séparées par 100 ms LOW.
static constexpr uint16_t C_LOCATOR_PULSE_MS = 100;
static constexpr uint8_t C_LOCATOR_PULSE_COUNT = 3;

enum class NodeType : uint8_t {
    ERM = 1,
    HUB = 2,
    DISTRIB = 3,
    CONVERTER = 4,
    BOOSTER = 5,
    LABOX = 6
};

enum class Mode : uint8_t {
    EXPLORATION = 1,
    OPERATION = 2,
    BLOCKED = 3
};

enum class Topology : uint8_t {
    WITH_HUB = 1,
    DIRECT = 2
};

enum class Opcode : uint8_t {
    HELLO = 0x01,
    ANNOUNCE_BEGIN = 0x02,
    DISCOVER_REQ = 0x03,
    ASSIGN_BEGIN = 0x05,
    ASSIGN_ACK = 0x06,
    RUN_PERMIT = 0x07,
    RUN_ACK = 0x08,
    ERROR = 0x09,
    DISCOVER_DONE = 0x0B,
    MODE_STATUS = 0x0C,
    SLOT_REQUEST = 0x0D,
    ANNOUNCE_MAC_0 = 0x10,
    ANNOUNCE_MAC_1 = 0x11,
    ASSIGN_MAC_0 = 0x12,
    ASSIGN_MAC_1 = 0x13,
    LABOX_REFERENCE_STATUS = 0x20,
    LABOX_REFERENCE_TIMING = 0x21,
    LABOX_REFERENCE_CUTOUT = 0x22
};

static constexpr uint8_t LABOX_REFERENCE_FORMAT_VERSION = 1;
static constexpr uint8_t LABOX_NODE_ID = 1;
static constexpr uint16_t LABOX_CAN_ID = 0x701;
static constexpr uint8_t LABOX_REFERENCE_FLAG_MAIN_ON = 0x01;
static constexpr uint8_t LABOX_REFERENCE_FLAG_RAILCOM_ENABLED = 0x02;
static constexpr uint8_t LABOX_REFERENCE_FLAG_RAILCOM_PAUSED = 0x04;
static constexpr uint8_t LABOX_REFERENCE_FLAG_TIMER_READY = 0x08;

enum class BoosterState : uint8_t {
    OK = 0,
    FAULT = 1,
    OVERHEAT = 2,
    OFF = 3
};

enum class ErrorCode : uint8_t {
    BAD_FRAME = 0x01,
    UNSUPPORTED_VERSION = 0x02,
    WRONG_MODE = 0x03,
    DUPLICATE_ID = 0x04,
    MAC_CONFLICT = 0x05,
    WRONG_SLOT = 0x06,
    UNKNOWN_NODE = 0x07,
    NO_RESPONSE = 0x08,
    ASSIGN_FAILED = 0x09,
    STORAGE_NOT_READY = 0x0A,
    TOPOLOGY_MISMATCH = 0x0B,
    SOURCE_ID_INVALID = 0x0C,
    INVENTORY_MISMATCH = 0x0D,
    RUN_DENIED = 0x0E,
    SESSION_MISMATCH = 0x0F
};

struct Identity {
    NodeType type;
    uint8_t id;
};

inline bool isAssignedIdentity(NodeType type, uint8_t id) {
    switch (type) {
    case NodeType::ERM:
    case NodeType::HUB:
        return id == 1;
    case NodeType::DISTRIB:
        return id >= 1 && id <= 8;
    case NodeType::CONVERTER: {
        const uint8_t district = id / 10;
        const uint8_t local = id % 10;
        return district >= 1 && district <= 8 && local >= 1 && local <= 8;
    }
    case NodeType::BOOSTER:
        return id >= 1 && id <= 252;
    case NodeType::LABOX:
        return id == 1;
    default:
        return false;
    }
}

inline bool classCode(NodeType type, uint8_t& code) {
    switch (type) {
    case NodeType::ERM:       code = 0; return true;
    case NodeType::HUB:       code = 1; return true;
    case NodeType::DISTRIB:   code = 2; return true;
    case NodeType::CONVERTER: code = 3; return true;
    case NodeType::LABOX:     code = 4; return true;
    default: return false; // Les boosters n’émettent pas sur le CAN découverte.
    }
}

inline bool makeArbitrationId(NodeType type, uint8_t id, uint16_t& canId) {
    uint8_t code = 0;
    if (!classCode(type, code) || !isAssignedIdentity(type, id)) return false;
    canId = static_cast<uint16_t>(CAN_ID_BASE +
                                  code * CAN_ID_CLASS_STRIDE + id);
    return canId <= CAN_ID_MAX_STANDARD;
}

inline bool makeUnassignedArbitrationId(NodeType type, uint16_t& canId) {
    uint8_t code = 0;
    if (!classCode(type, code)) return false;
    canId = static_cast<uint16_t>(CAN_ID_BASE +
                                  code * CAN_ID_CLASS_STRIDE + ID_UNASSIGNED);
    return canId <= CAN_ID_MAX_STANDARD;
}

inline bool decodeArbitrationId(uint16_t canId, Identity& identity) {
    if (canId < CAN_ID_BASE || canId > CAN_ID_BASE + 5 * CAN_ID_CLASS_STRIDE - 1)
        return false;
    const uint16_t relative = canId - CAN_ID_BASE;
    const uint8_t code = static_cast<uint8_t>(relative / CAN_ID_CLASS_STRIDE);
    const uint8_t id = static_cast<uint8_t>(relative % CAN_ID_CLASS_STRIDE);
    if (id == ID_UNASSIGNED) return false; // Adresse transitoire, pas une identité.
    switch (code) {
    case 0: identity.type = NodeType::ERM; break;
    case 1: identity.type = NodeType::HUB; break;
    case 2: identity.type = NodeType::DISTRIB; break;
    case 3: identity.type = NodeType::CONVERTER; break;
    case 4: identity.type = NodeType::LABOX; break;
    default: return false;
    }
    identity.id = id;
    return isAssignedIdentity(identity.type, identity.id);
}

inline bool isValidControlFrame(uint8_t dlc, const uint8_t* data);

// Verrou local de sécurité H/D : les sorties sont autorisées seulement si un
// HELLO S1 récent annonce l’Exploitation et qu’un RUN_PERMIT S1 récent,
// cohérent avec la topologie, a été reçu.
class RunPermitGuard {
public:
    explicit RunPermitGuard(Topology expectedTopology,
                            bool enforceExpectedTopology = true)
        : topology_(expectedTopology), enforceTopology_(enforceExpectedTopology) {}

    bool onFrame(uint16_t canId, uint8_t dlc, const uint8_t* data,
                 uint32_t nowMs) {
        Identity source{};
        if (!isValidControlFrame(dlc, data) ||
            !decodeArbitrationId(canId, source) ||
            source.type != NodeType::ERM || source.id != 1)
            return false;

        const uint16_t transaction = static_cast<uint16_t>(data[2]) |
            (static_cast<uint16_t>(data[3]) << 8);
        if (data[0] == static_cast<uint8_t>(Opcode::HELLO)) {
            if (transaction != 0 ||
                data[4] != static_cast<uint8_t>(NodeType::ERM) ||
                data[5] != 1 || data[6] < static_cast<uint8_t>(Mode::EXPLORATION) ||
                data[6] > static_cast<uint8_t>(Mode::BLOCKED) ||
                data[7] < static_cast<uint8_t>(Topology::WITH_HUB) ||
                data[7] > static_cast<uint8_t>(Topology::DIRECT))
                return false;
            const Mode newMode = static_cast<Mode>(data[6]);
            const Topology newTopology = static_cast<Topology>(data[7]);
            if (newMode != Mode::OPERATION ||
                (enforceTopology_ && newTopology != topology_) ||
                mode_ != newMode || topologyFromS_ != newTopology)
                permitSeen_ = false;
            mode_ = newMode;
            topologyFromS_ = newTopology;
            helloAtMs_ = nowMs;
            helloSeen_ = true;
            return true;
        }

        if (data[0] == static_cast<uint8_t>(Opcode::RUN_PERMIT)) {
            if (transaction == 0 || !helloSeen_ ||
                mode_ != Mode::OPERATION || topologyFromS_ !=
                    static_cast<Topology>(data[6]) ||
                (enforceTopology_ && topologyFromS_ != topology_) ||
                nowMs - helloAtMs_ >= NODE_LOSS_TIMEOUT_MS ||
                data[4] != static_cast<uint8_t>(NodeType::ERM) ||
                data[5] != 1 ||
                (enforceTopology_ && data[6] != static_cast<uint8_t>(topology_)) ||
                data[7] == 0 ||
                static_cast<uint32_t>(data[7]) * 10U > RUN_PERMIT_TTL_MS)
                return false;
            permitTtlMs_ = static_cast<uint32_t>(data[7]) * 10U;
            permitAtMs_ = nowMs;
            permitSeen_ = true;
            return true;
        }
        return false;
    }

    bool allows(Topology localTopology, uint32_t nowMs) const {
        return helloSeen_ && permitSeen_ &&
               (!enforceTopology_ || localTopology == topology_) &&
               mode_ == Mode::OPERATION && topologyFromS_ == localTopology &&
               nowMs - helloAtMs_ < NODE_LOSS_TIMEOUT_MS &&
               nowMs - permitAtMs_ < permitTtlMs_;
    }

    Mode mode() const { return mode_; }
    Topology topology() const { return topologyFromS_; }
    bool hasFreshHello(uint32_t nowMs) const {
        return helloSeen_ && nowMs - helloAtMs_ < NODE_LOSS_TIMEOUT_MS;
    }

private:
    Topology topology_;
    bool enforceTopology_;
    Mode mode_ = Mode::BLOCKED;
    Topology topologyFromS_ = Topology::DIRECT;
    uint32_t helloAtMs_ = 0;
    uint32_t permitAtMs_ = 0;
    uint32_t permitTtlMs_ = 0;
    bool helloSeen_ = false;
    bool permitSeen_ = false;
};

inline bool isValidControlFrame(uint8_t dlc, const uint8_t* data) {
    return dlc == CONTROL_DLC && data != nullptr &&
           data[1] == PROTOCOL_VERSION_MAJOR;
}

inline uint8_t locatorCrc8(const uint8_t* data, size_t length) {
    uint8_t crc = 0;
    if (data == nullptr) return crc;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit)
            crc = (crc & 0x80U) ? static_cast<uint8_t>((crc << 1) ^ 0x07U)
                                : static_cast<uint8_t>(crc << 1);
    }
    return crc;
}

inline void makeDLocatorPayload(const uint8_t mac[6], uint8_t payload[7]) {
    if (mac == nullptr || payload == nullptr) return;
    for (uint8_t i = 0; i < 6; ++i) payload[i] = mac[i];
    payload[6] = locatorCrc8(mac, 6);
}

inline bool isValidDLocatorPayload(const uint8_t payload[7]) {
    return payload != nullptr && locatorCrc8(payload, 6) == payload[6];
}

inline bool isValidSlotRequest(const uint8_t payload[4], uint8_t sourceDId) {
    if (payload == nullptr || sourceDId < 1 || sourceDId > 8) return false;
    const uint8_t targetType = payload[0];
    const uint8_t localPort = payload[1];
    const uint8_t expectedCId = payload[2];
    return targetType == static_cast<uint8_t>(NodeType::CONVERTER) &&
           localPort >= 1 && localPort <= 8 &&
           expectedCId == static_cast<uint8_t>(sourceDId * 10 + localPort) &&
           payload[3] == 0;
}

inline bool isRelayedBoosterTelemetry(uint16_t canId, uint8_t dlc,
                                      const uint8_t* data,
                                      Identity* source = nullptr) {
    Identity decoded{};
    if (dlc != TELEMETRY_B_RELAY_DLC || data == nullptr ||
        data[1] != static_cast<uint8_t>(NodeType::BOOSTER) ||
        !isAssignedIdentity(NodeType::BOOSTER, data[0]) ||
        data[4] > static_cast<uint8_t>(BoosterState::OFF) ||
        !decodeArbitrationId(canId, decoded) ||
        decoded.type != NodeType::CONVERTER || decoded.id != data[5])
        return false;
    if (source != nullptr) *source = decoded;
    return true;
}

inline bool isDiscoveryTelemetry(uint16_t canId, uint8_t dlc,
                                 const uint8_t* data,
                                 Identity* source = nullptr) {
    if (data == nullptr) return false;
    if (dlc == TELEMETRY_B_RELAY_DLC)
        return isRelayedBoosterTelemetry(canId, dlc, data, source);
    if (dlc != TELEMETRY_B_LOCAL_DLC) return false;
    Identity decoded{};
    if (!decodeArbitrationId(canId, decoded) ||
        decoded.type == NodeType::ERM || decoded.type == NodeType::BOOSTER ||
        data[1] != static_cast<uint8_t>(decoded.type) ||
        data[0] != decoded.id || data[4] > static_cast<uint8_t>(BoosterState::OFF))
        return false;
    if (source != nullptr) *source = decoded;
    return true;
}

} // namespace discovery
} // namespace cfc
