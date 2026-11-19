#pragma once

#include <stddef.h>
#include <stdint.h>

// Valeurs partagées du CAN de découverte CFC, version de départ 0.1.0.
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

static constexpr uint32_t S_HELLO_PERIOD_MS = 500;
static constexpr uint32_t RUN_PERMIT_PERIOD_MS = 500;
static constexpr uint32_t RUN_PERMIT_TTL_MS = 1500;
static constexpr uint32_t NODE_LOSS_TIMEOUT_MS = 1500;
static constexpr uint32_t DISCOVERY_RESPONSE_TIMEOUT_MS = 500;
static constexpr uint32_t ANNOUNCE_FRAGMENT_TIMEOUT_MS = 500;
static constexpr uint32_t ABSENCE_INTERVENTION_MS = 600000;

enum class NodeType : uint8_t {
    ERM = 1,
    HUB = 2,
    DISTRIB = 3,
    CONVERTER = 4,
    BOOSTER = 5
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
    ANNOUNCE_MAC_0 = 0x10,
    ANNOUNCE_MAC_1 = 0x11,
    ASSIGN_MAC_0 = 0x12,
    ASSIGN_MAC_1 = 0x13
};

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
    if (canId < CAN_ID_BASE || canId > CAN_ID_BASE + 4 * CAN_ID_CLASS_STRIDE - 1)
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
    default: return false;
    }
    identity.id = id;
    return isAssignedIdentity(identity.type, identity.id);
}

inline bool isValidControlFrame(uint8_t dlc, const uint8_t* data) {
    return dlc == CONTROL_DLC && data != nullptr &&
           data[1] == PROTOCOL_VERSION_MAJOR;
}

inline bool isRelayedBoosterTelemetry(uint16_t canId, uint8_t dlc,
                                      const uint8_t* data,
                                      Identity* source = nullptr) {
    Identity decoded{};
    if (dlc != TELEMETRY_B_RELAY_DLC || data == nullptr ||
        data[1] != static_cast<uint8_t>(NodeType::BOOSTER) ||
        !decodeArbitrationId(canId, decoded) ||
        decoded.type != NodeType::CONVERTER || decoded.id != data[5])
        return false;
    if (source != nullptr) *source = decoded;
    return true;
}

} // namespace discovery
} // namespace cfc
