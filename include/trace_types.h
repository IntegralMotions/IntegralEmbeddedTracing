#pragma once

#include <cstddef>
#include <cstdint>

enum class TraceMessageType : uint8_t {
    CONFIG = 0x0,
    ARRAY_CONFIG = 0x1,
    UPDATE = 0x2,
    ARRAY_UPDATE = 0x3,
};

enum class TraceValueType : uint8_t {
    UNKNOWN = 0x0,
    CHAR = 0x1,
    UCHAR = 0x2,
    INT8 = 0x3,
    UINT8 = 0x4,
    INT16 = 0x5,
    UINT16 = 0x6,
    INT32 = 0x7,
    UINT32 = 0x8,
    INT64 = 0x9,
    UINT64 = 0xA,
    FLOAT = 0xB,
    DOUBLE = 0xC,
    LONG_DOUBLE = 0xD,
};

inline size_t getTraceValueTypeSize(TraceValueType type) {
    switch (type) {
    case TraceValueType::CHAR:
        return sizeof(char);
    case TraceValueType::UCHAR:
        return sizeof(unsigned char);
    case TraceValueType::INT8:
        return sizeof(int8_t);
    case TraceValueType::UINT8:
        return sizeof(uint8_t);
    case TraceValueType::INT16:
        return sizeof(int16_t);
    case TraceValueType::UINT16:
        return sizeof(uint16_t);
    case TraceValueType::INT32:
        return sizeof(int32_t);
    case TraceValueType::UINT32:
        return sizeof(uint32_t);
    case TraceValueType::INT64:
        return sizeof(int64_t);
    case TraceValueType::UINT64:
        return sizeof(uint64_t);
    case TraceValueType::FLOAT:
        return sizeof(float);
    case TraceValueType::DOUBLE:
        return sizeof(double);
    case TraceValueType::LONG_DOUBLE:
        return sizeof(long double);
    case TraceValueType::UNKNOWN:
    default:
        return 0; // Unknown type
    }
}

template <typename T> TraceValueType getTraceValueType();

template <typename T> inline TraceValueType getTraceValueType() {
    return TraceValueType::UNKNOWN;
}

template <> inline TraceValueType getTraceValueType<int8_t>() {
    return TraceValueType::INT8;
}

template <> inline TraceValueType getTraceValueType<uint8_t>() {
    return TraceValueType::UINT8;
}

template <> inline TraceValueType getTraceValueType<int16_t>() {
    return TraceValueType::INT16;
}

template <> inline TraceValueType getTraceValueType<uint16_t>() {
    return TraceValueType::UINT16;
}

template <> inline TraceValueType getTraceValueType<int32_t>() {
    return TraceValueType::INT32;
}

template <> inline TraceValueType getTraceValueType<uint32_t>() {
    return TraceValueType::UINT32;
}

template <> inline TraceValueType getTraceValueType<int64_t>() {
    return TraceValueType::INT64;
}

template <> inline TraceValueType getTraceValueType<uint64_t>() {
    return TraceValueType::UINT64;
}

template <> inline TraceValueType getTraceValueType<float>() {
    return TraceValueType::FLOAT;
}

template <> inline TraceValueType getTraceValueType<double>() {
    return TraceValueType::DOUBLE;
}

template <> inline TraceValueType getTraceValueType<long double>() {
    return TraceValueType::LONG_DOUBLE;
}
