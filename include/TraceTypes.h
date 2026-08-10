#pragma once

#include <cstddef>
#include <cstdint>

namespace IntegralMotions::Tracing {
    enum class TraceMessageType : uint8_t {
        Config = 0x0,
        ArrayConfig = 0x1,
        Update = 0x2,
        ArrayUpdate = 0x3,
    };

    enum class TraceProtocolMessageType : uint8_t {
        GetConfigRequest = 0x01,
        GetConfigResponse = 0x02,
        GetArrayConfigRequest = 0x03,
        GetArrayConfigResponse = 0x04,
        StartTraceRequest = 0x05,
        StartTraceResponse = 0x06,
        TraceData = 0x07,
        StopTraceEvent = 0x08,
    };

    enum class TraceValueType : uint8_t {
        Unknown = 0x0,
        Char = 0x1,
        UChar = 0x2,
        Int8 = 0x3,
        UInt8 = 0x4,
        Int16 = 0x5,
        UInt16 = 0x6,
        Int32 = 0x7,
        UInt32 = 0x8,
        Int64 = 0x9,
        UInt64 = 0xA,
        Float = 0xB,
        Double = 0xC,
        LongDouble = 0xD,
    };

    inline size_t getTraceValueTypeSize(TraceValueType type) {
        switch (type) {
        case TraceValueType::Char:
            return sizeof(char);
        case TraceValueType::UChar:
            return sizeof(unsigned char);
        case TraceValueType::Int8:
            return sizeof(int8_t);
        case TraceValueType::UInt8:
            return sizeof(uint8_t);
        case TraceValueType::Int16:
            return sizeof(int16_t);
        case TraceValueType::UInt16:
            return sizeof(uint16_t);
        case TraceValueType::Int32:
            return sizeof(int32_t);
        case TraceValueType::UInt32:
            return sizeof(uint32_t);
        case TraceValueType::Int64:
            return sizeof(int64_t);
        case TraceValueType::UInt64:
            return sizeof(uint64_t);
        case TraceValueType::Float:
            return sizeof(float);
        case TraceValueType::Double:
            return sizeof(double);
        case TraceValueType::LongDouble:
            return sizeof(long double);
        case TraceValueType::Unknown:
        default:
            return 0; // Unknown type
        }
    }

    template <typename T>
    TraceValueType getTraceValueType();

    template <typename T>
    inline TraceValueType getTraceValueType() {
        return TraceValueType::Unknown;
    }

    template <>
    inline TraceValueType getTraceValueType<int8_t>() {
        return TraceValueType::Int8;
    }

    template <>
    inline TraceValueType getTraceValueType<uint8_t>() {
        return TraceValueType::UInt8;
    }

    template <>
    inline TraceValueType getTraceValueType<int16_t>() {
        return TraceValueType::Int16;
    }

    template <>
    inline TraceValueType getTraceValueType<uint16_t>() {
        return TraceValueType::UInt16;
    }

    template <>
    inline TraceValueType getTraceValueType<int32_t>() {
        return TraceValueType::Int32;
    }

    template <>
    inline TraceValueType getTraceValueType<uint32_t>() {
        return TraceValueType::UInt32;
    }

    template <>
    inline TraceValueType getTraceValueType<int64_t>() {
        return TraceValueType::Int64;
    }

    template <>
    inline TraceValueType getTraceValueType<uint64_t>() {
        return TraceValueType::UInt64;
    }

    template <>
    inline TraceValueType getTraceValueType<float>() {
        return TraceValueType::Float;
    }

    template <>
    inline TraceValueType getTraceValueType<double>() {
        return TraceValueType::Double;
    }

    template <>
    inline TraceValueType getTraceValueType<long double>() {
        return TraceValueType::LongDouble;
    }
} // namespace IntegralMotions::Tracing