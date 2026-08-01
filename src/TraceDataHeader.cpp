#include "TraceDataHeader.h"

namespace {
    constexpr uint8_t OneByteValueSize = 1;
    constexpr uint8_t TwoByteValueSize = 2;
    constexpr uint8_t FourByteValueSize = 4;
    constexpr uint8_t EightByteValueSize = 8;
} // namespace

namespace IntegralMotions::Tracing {
    TraceDataSizeCode getTraceDataSizeCode(uint8_t typeSize) {
        switch (typeSize) {
        case OneByteValueSize:
            return TraceDataSizeCode::OneByte;
        case TwoByteValueSize:
            return TraceDataSizeCode::TwoBytes;
        case FourByteValueSize:
            return TraceDataSizeCode::FourBytes;
        case EightByteValueSize:
            return TraceDataSizeCode::EightBytes;
        default:
            return TraceDataSizeCode::OneByte;
        }
    }
} // namespace IntegralMotions::Tracing