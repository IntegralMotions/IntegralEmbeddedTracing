#include "trace_header.h"
#include <cstdint>

inline constexpr uint8_t HeaderMessageTypeMask = 0x03;
inline constexpr uint8_t HeaderMessageTypeShift = 6;
inline constexpr uint8_t HeaderMessageTypeByte = 0;

inline constexpr uint8_t HeaderIdHighMask = 0x3F;
inline constexpr uint8_t HeaderIdHighShift = 4;
inline constexpr uint8_t HeaderIdHighByte = 0;
inline constexpr uint8_t HeaderIdLowMask = 0x0F;
inline constexpr uint8_t HeaderIdLowShift = 4;
inline constexpr uint8_t HeaderIdLowByte = 1;

inline constexpr uint8_t HeaderValueTypeMask = 0x0F;
inline constexpr uint8_t HeaderValueTypeByte = 1;

TraceHeader getTraceHeaderFromBytes(const uint8_t* bytes) {
    TraceHeader header;
    header.messageType =
        (TraceMessageType) ((bytes[HeaderMessageTypeByte] >> HeaderMessageTypeShift) & HeaderMessageTypeMask);
    header.id = ((bytes[HeaderIdHighByte] & HeaderIdHighMask) << HeaderIdHighShift) |
                ((bytes[HeaderIdLowByte] >> HeaderIdLowShift) & HeaderIdLowMask);
    header.valueType = (TraceValueType) (bytes[HeaderValueTypeByte] & HeaderValueTypeMask);
    return header;
}

void getBytesFromTraceHeader(const TraceHeader& header, uint8_t* bytes) {
    bytes[HeaderMessageTypeByte] =
        ((int) header.messageType << HeaderMessageTypeShift) | ((header.id >> HeaderIdHighShift) & HeaderIdHighMask);
    bytes[HeaderValueTypeByte] =
        ((header.id & HeaderIdLowMask) << HeaderIdLowShift) | ((int) header.valueType & HeaderValueTypeMask);
}
