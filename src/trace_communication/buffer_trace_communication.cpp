#include "trace_communication/buffer_trace_communication.h"
#include "seven_bit_encoding.h"
#include "fast_copy.h"

BufferTraceCommunication::BufferTraceCommunication(const size_t bufferSize) :
		BUFFER_SIZE(bufferSize), _writeBuffer(new uint8_t[BUFFER_SIZE]), _flushBuffer(new uint8_t[BUFFER_SIZE]) {
}

size_t BufferTraceCommunication::getBufferIndex() {
	return _writeBufferIndex;
}

void BufferTraceCommunication::write(uint8_t *data, uint8_t dataSize) {
	if (_writeBufferIndex + dataSize > BUFFER_SIZE) {
		return;
	}

	fast_copy(_writeBuffer + _writeBufferIndex, data, dataSize);
	_writeBufferIndex += dataSize;
}
