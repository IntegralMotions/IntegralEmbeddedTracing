#include "trace_communication/trace_communication.h"
#include "seven_bit_encoding.h"
#include <cstring>

TraceCommunication::TraceCommunication() { }
TraceCommunication::~TraceCommunication() { }

void TraceCommunication::writeTraceMessage(const TraceHeader &header, const uint8_t* value, const uint8_t valueSize) {
    uint8_t toSend[18];
    getBytesFromTraceHeader(header, toSend);
    memcpy(&toSend[2], value, valueSize);

	size_t encodedSize = SevenBitEncoding::getEncodedBufferSize(2 + valueSize);
	uint8_t encoded[encodedSize];
	size_t actualEncodedSize = SevenBitEncoding::encodeBuffer(toSend, 2 + valueSize, encoded);

    write(encoded, actualEncodedSize);
}

void TraceCommunication::writeArrayTraceMessage(const TraceHeader &header, uint32_t index, const uint8_t* value, const uint8_t valueSize) {
    size_t lenSize = SevenBitEncoding::getEncodedSize(index);
    uint8_t toSend[23];
    getBytesFromTraceHeader(header, toSend);
    SevenBitEncoding::encodeValue(index, toSend + 2);
    memcpy(toSend + 2 + lenSize, value, valueSize);

	size_t encodedSize = SevenBitEncoding::getEncodedBufferSize(2 + lenSize + valueSize);
	uint8_t encoded[encodedSize];
	size_t actualEncodedSize = SevenBitEncoding::encodeBuffer(toSend, 2 + lenSize + valueSize, encoded);

    write(encoded, actualEncodedSize);
}

bool TraceCommunication::tryReadTraceMessage(TraceHeader& header, uint8_t* value) {
    if (_receivingState == WAITING_FOR_HEADER) {
        if (tryRead(rxBuffer, 2)) {
            _currentHeader = getTraceHeaderFromBytes(rxBuffer);
            if (_currentHeader.messageType == TraceMessageType::CONFIG ||
                _currentHeader.messageType == TraceMessageType::ARRAY_CONFIG) {
                header = _currentHeader;
                return true;
            }
            arraySizeReadIndex = 0;
            _receivingState = WAITING_FOR_DATA;
        }
    }
    else if (_receivingState == WAITING_FOR_DATA) {
        if (_currentHeader.messageType == TraceMessageType::ARRAY_UPDATE) {
            if (arraySizeReadIndex > 0 && (rxBuffer[arraySizeReadIndex - 1] & 0x80) != 0) {
                if (tryRead(rxBuffer + arraySizeReadIndex, 1))
                    arraySizeReadIndex++;
            }
            else {
                uint8_t dataSize = getTraceValueTypeSize(_currentHeader.valueType);
                if (tryRead(rxBuffer + arraySizeReadIndex, dataSize)) {
                    _receivingState = WAITING_FOR_HEADER;
                    header = _currentHeader;
                    memcpy(value, rxBuffer, arraySizeReadIndex + dataSize);
                    return true;
                }
            }
        }
        else {
            uint8_t dataSize = getTraceValueTypeSize(_currentHeader.valueType);
            if (tryRead(rxBuffer, dataSize)) {
                _receivingState = WAITING_FOR_HEADER;
                header = _currentHeader;
                memcpy(value, rxBuffer, dataSize);
                return true;
            }
        }
    }
    return false;
}
