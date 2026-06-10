#include "trace_header.h"

TraceHeader getTraceHeaderFromBytes(const uint8_t* bytes)
{
	TraceHeader header;
	header.messageType = (TraceMessageType)((bytes[0] >> 6) & 0x03);
	header.id = ((bytes[0] & 0x3F) << 4) | ((bytes[1] >> 4) & 0x0F);
	header.valueType = (TraceValueType)(bytes[1] & 0x0F);
	return header;
}

void getBytesFromTraceHeader(const TraceHeader &header, uint8_t* bytes)
{
	bytes[0] = ((int)header.messageType << 6) | ((header.id >> 4) & 0x3F);
	bytes[1] = ((header.id & 0x0F) << 4) | ((int)header.valueType & 0x0F);
}
