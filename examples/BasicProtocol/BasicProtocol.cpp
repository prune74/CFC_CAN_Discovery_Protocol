#include <Arduino.h>
#include <CfcDiscoveryProtocol.h>

using namespace cfc::discovery;

void setup() {
    Serial.begin(115200);

    uint16_t canId = 0;
    if (makeArbitrationId(NodeType::CONVERTER, 11, canId)) {
        Serial.printf("Convertisseur C11 : CAN ID 0x%03X\n", canId);
    }

    Identity source{};
    if (decodeArbitrationId(0x68B, source)) {
        Serial.printf("Source décodée : type %u, ID %u\n",
                      static_cast<unsigned>(source.type), source.id);
    }
}

void loop() {}
