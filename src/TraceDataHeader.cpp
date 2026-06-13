#include "IntegralEmbeddedTracing/TraceDataHeader.h"

namespace {
    constexpr uint8_t OneByteValueSize = 1;
    constexpr uint8_t TwoByteValueSize = 2;
    constexpr uint8_t FourByteValueSize = 4;
    constexpr uint8_t EightByteValueSize = 8;
} // namespace

TraceDataSizeCode getTraceDataSizeCode(uint8_t typeSize) {
    switch (typeSize) {
    case OneByteValueSize:
        return TraceDataSizeCode::ONE_BYTE;
    case TwoByteValueSize:
        return TraceDataSizeCode::TWO_BYTES;
    case FourByteValueSize:
        return TraceDataSizeCode::FOUR_BYTES;
    case EightByteValueSize:
        return TraceDataSizeCode::EIGHT_BYTES;
    default:
        return TraceDataSizeCode::ONE_BYTE;
    }
}
