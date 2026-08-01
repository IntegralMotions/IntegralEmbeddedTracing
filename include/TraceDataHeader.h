#pragma once

#include <cstdint>

namespace IntegralMotions::Tracing {
    enum class TraceDataSizeCode : uint8_t {
        OneByte = 0,
        TwoBytes = 1,
        FourBytes = 2,
        EightBytes = 3,
    };

    struct TraceDataHeader {
        uint8_t variableId : 6;
        TraceDataSizeCode sizeCode : 2;
    };

    TraceDataSizeCode getTraceDataSizeCode(uint8_t typeSize);
} // namespace IntegralMotions::Tracing