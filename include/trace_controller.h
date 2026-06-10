#pragma once
#include "trace/array_trace.h"
#include "trace/variable_trace.h"
#include "trace_communication/trace_communication.h"
#include "trace_types.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>

#define MAX_TRACED_VARIABLES 1024

class TraceController {
public:
	static void init(TraceCommunication* comm);
	static TraceController& get();

    template<typename T>
    void addVariable(const char* name, T* ref);
    template<typename T>
    void addArray(const char* name, T* ref, uint32_t length);

    void loop();
    void checkForMessage();
    void checkForUpdates();

private:
    TraceController(TraceCommunication* comm);
    ~TraceController() = default;
    TraceController(const TraceController&) = delete;
    TraceController& operator=(const TraceController&) = delete;

    void sendConfig();
    void sendArrayConfig(const ArrayTrace* traces, uint16_t count);

    TraceCommunication* _communicator;
    uint16_t _amountOfTraces = 0;
    uint16_t _amountOfVariableTraces = 0;
    uint16_t _amountOfArrayTraces = 0;
    VariableTrace _variables[MAX_TRACED_VARIABLES];
    ArrayTrace _arrays[MAX_TRACED_VARIABLES];
    VariableTrace* _variableLookup[MAX_TRACED_VARIABLES] = { nullptr };
    ArrayTrace* _arrayLookup[MAX_TRACED_VARIABLES] = { nullptr };
    bool _sendingConfig = false;
    bool _configSend = false;

    static TraceController* _instance;
};

template<typename T>
void TraceController::addVariable(const char* name, T* ref) {
    if (_amountOfTraces >= MAX_TRACED_VARIABLES) return;
    VariableTrace trace;
    trace.name = name;
    trace.id = _amountOfTraces++;
    trace.type = getTraceValueType<T>();
    trace.typeSize = getTraceValueTypeSize(trace.type);
    trace.value = reinterpret_cast<uint8_t*>(ref);
    trace.previousValue = reinterpret_cast<uint8_t*>(malloc(sizeof(T)));
    trace.updateHeader = { trace.id, TraceMessageType::UPDATE, trace.type };
    _variables[_amountOfVariableTraces] = trace;
    _variableLookup[trace.id] = &_variables[_amountOfVariableTraces];
    _amountOfVariableTraces++;
}

template<typename T>
void TraceController::addArray(const char* name, T* ref, uint32_t length) {
    if (_amountOfTraces >= MAX_TRACED_VARIABLES) return;
    ArrayTrace trace;
    trace.name = name;
    trace.id = _amountOfTraces++;
    trace.length = length;
    trace.value = reinterpret_cast<uint8_t*>(ref);
    _arrays[_amountOfArrayTraces] = trace;
    _arrayLookup[trace.id] = &_arrays[_amountOfArrayTraces];
    _amountOfArrayTraces++;
}
