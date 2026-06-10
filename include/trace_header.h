#pragma once
#include "trace_types.h"
#include <inttypes.h>

struct TraceHeader {
	uint16_t id :12;
	TraceMessageType messageType;
	TraceValueType valueType;
};

TraceHeader getTraceHeaderFromBytes(const uint8_t *bytes);
void getBytesFromTraceHeader(const TraceHeader &header, uint8_t *bytes);
