#include "trace_controller.h"
#include "seven_bit_encoding.h"
#include "fast_copy.h"

TraceController *TraceController::_instance = nullptr;

void TraceController::init(TraceCommunication *comm) {
	if (!_instance)
		_instance = new TraceController(comm);
}

TraceController& TraceController::get() {
	return *_instance;
}

TraceController::TraceController(TraceCommunication *comm) :
		_communicator(comm) {
}

void TraceController::loop() {
	if (_sendingConfig) {
		return;
	}

	checkForMessage();
	checkForUpdates();
}

void TraceController::checkForMessage() {
	TraceHeader header;
	uint8_t valueBuffer[21];
	if (!_communicator->tryReadTraceMessage(header, valueBuffer))
		return;

	if (header.messageType == TraceMessageType::CONFIG)
		sendConfig();
	else if (header.messageType == TraceMessageType::ARRAY_CONFIG)
		sendArrayConfig(_arrays, _amountOfArrayTraces);
	else if (header.messageType == TraceMessageType::UPDATE) {
		if (header.id < MAX_TRACED_VARIABLES && _variableLookup[header.id])
			fast_copy(_variableLookup[header.id]->value, valueBuffer,
					getTraceValueTypeSize(_variableLookup[header.id]->type));
	} else if (header.messageType == TraceMessageType::ARRAY_UPDATE) {
		if (header.id < MAX_TRACED_VARIABLES && _arrayLookup[header.id]) {
			uint8_t dataSize = getTraceValueTypeSize(
					_arrayLookup[header.id]->type);
			size_t consumed = 0;
			SevenBitEncoding::decodeValue(valueBuffer, 5, consumed);
			fast_copy(_arrayLookup[header.id]->value, valueBuffer + consumed,
					dataSize);
		}
	}
}

void TraceController::checkForUpdates() {
	if (!_configSend)
		return;

	for (uint16_t i = 0; i < _amountOfVariableTraces; ++i) {
		const auto &trace = _variables[i];
		if (memcmp(trace.value, trace.previousValue, trace.typeSize) != 0) {
			_communicator->writeTraceMessage(trace.updateHeader, trace.value,
					trace.typeSize);

			memcpy(trace.previousValue, trace.value, trace.typeSize);
		}
	}
}

void TraceController::sendConfig() {
	_sendingConfig = true;

	for (uint16_t i = 0; i < _amountOfVariableTraces; i++) {
		const auto &trace = _variables[i];
		TraceHeader header { trace.id, TraceMessageType::CONFIG, trace.type };
		_communicator->writeTraceMessage(header,
				reinterpret_cast<const uint8_t*>(trace.name),
				strlen(trace.name) + 1);
	}

	_sendingConfig = false;
	_configSend = true;
}

void TraceController::sendArrayConfig(const ArrayTrace *traces,
		uint16_t count) {
	for (uint16_t i = 0; i < count; i++) {
		const auto &trace = traces[i];
		TraceHeader header { trace.id, TraceMessageType::ARRAY_CONFIG,
				TraceValueType::UNKNOWN };
		_communicator->writeArrayTraceMessage(header, trace.length,
				reinterpret_cast<const uint8_t*>(trace.name),
				strlen(trace.name) + 1);
		for (uint32_t index = 0; index < trace.length; index++)
			trace.update(index);
	}
}
