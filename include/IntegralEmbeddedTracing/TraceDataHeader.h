#pragma once

#include <cstdint>

enum class TraceDataSizeCode : uint8_t {
    ONE_BYTE = 0,
    TWO_BYTES = 1,
    FOUR_BYTES = 2,
    EIGHT_BYTES = 3,
};

struct TraceDataHeader {
    uint8_t variableId : 6;
    TraceDataSizeCode sizeCode : 2;
};

TraceDataSizeCode getTraceDataSizeCode(uint8_t typeSize);
