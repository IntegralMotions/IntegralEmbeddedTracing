#pragma once

#include "trace_communication/trace_communication.h"

class BufferTraceCommunication: public TraceCommunication {
public:
	BufferTraceCommunication(const size_t bufferSize);

	virtual void flush() = 0;
	size_t getBufferIndex();

private:
	virtual void write(uint8_t *data, const uint8_t dataSize) override;

protected:
	const size_t BUFFER_SIZE = 2046;
	size_t _writeBufferIndex = 0;
	uint8_t *_writeBuffer;
	uint8_t *_flushBuffer;
};
