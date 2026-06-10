#pragma once
#include "trace_header.h"

enum MessageReceiveState {
	WAITING_FOR_HEADER = 0,
	WAITING_FOR_DATA = 2,
};

class TraceCommunication
{
public:
	TraceCommunication();
	virtual ~TraceCommunication();

	virtual void writeTraceMessage(const TraceHeader &header, const uint8_t* value, const uint8_t value_size);
	virtual void writeArrayTraceMessage(const TraceHeader &header, const uint32_t index, const uint8_t* value, const uint8_t value_size);
	virtual bool tryReadTraceMessage(TraceHeader& header, uint8_t* value);

private:
	virtual void write(uint8_t* data, const uint8_t data_size) = 0;
	virtual bool tryRead(uint8_t* data, const uint8_t data_size) = 0;
private:
	MessageReceiveState _receivingState = WAITING_FOR_HEADER;
	TraceHeader _currentHeader;

	uint8_t rxBuffer[5 + 16];
	uint8_t arraySizeReadIndex = 0;
};
