#pragma once
#include <IntegralCommunication/CobsEncodedCommunication.h>
#include <IntegralCommunication/Communication.h>
#include <IntegralCommunication/Messaging/Message.h>
#include <array>
#include <cstdint>
#include <cstring>

#include "Trace/ArrayTrace.h"
#include "Trace/VariableTrace.h"
#include "TraceDataHeader.h"
#include "TraceTypes.h"

namespace IntegralMotions::Tracing {
    class TraceController {
      public:
        static constexpr uint16_t maxTracedVariables = 256;
        static constexpr uint8_t maxActiveTraceVariables = 64;

        static void init(Communication& comm);
        static TraceController& get();

        TraceController(const TraceController&) = delete;
        TraceController& operator=(const TraceController&) = delete;
        TraceController(TraceController&&) = delete;
        TraceController& operator=(TraceController&&) = delete;

        template <typename T>
        void addVariable(const char* name, T* ref);
        template <typename T>
        void addArray(const char* name, T* ref, uint32_t length);

        void loop();
        void checkForMessage();
        void checkForUpdates();

      private:
        TraceController(Communication& comm);
        ~TraceController() = default;

        void sendGetConfigResponse(const Message& message);
        void sendGetArrayConfigResponse(const Message& message);
        void sendStartTraceResponse(const Message& message);
        void handleIncomingMessage(const Message& message);

        CobsEncodedCommunication _framedCommunication;
        bool _tracingStarted = false;
        uint16_t _amountOfTraces = 0;
        uint16_t _amountOfVariableTraces = 0;
        uint16_t _amountOfArrayTraces = 0;
        std::array<VariableTrace, maxTracedVariables> _variables = {};
        std::array<ArrayTrace, maxTracedVariables> _arrays = {};
        std::array<uint8_t*, maxTracedVariables> _previousValueStorage = {nullptr};
        std::array<Trace*, maxTracedVariables> _traceTableLookup = {nullptr};
        std::array<uint8_t, maxActiveTraceVariables> _tracedVariableIds = {};
        uint8_t _amountOfTracedVariables = 0;
        bool _variableConfigSend = false;
        bool _arrayConfigSend = false;

        static TraceController* instance;
    };

    template <typename T>
    void TraceController::addVariable(const char* name, T* ref) {
        if (_amountOfTraces >= maxTracedVariables) {
            return;
        }
        VariableTrace trace;
        trace.name = name;
        trace.type = getTraceValueType<T>();
        if (trace.type == TraceValueType::Unknown) {
            return;
        }

        trace.typeSize = getTraceValueTypeSize(trace.type);
        trace.id = _amountOfTraces++;
        trace.value = reinterpret_cast<uint8_t*>(ref);
        _previousValueStorage[trace.id] = new uint8_t[trace.typeSize];
        trace.previousValue = _previousValueStorage[trace.id];
        trace.updateHeader.variableId = static_cast<uint8_t>(trace.id);
        trace.updateHeader.sizeCode = getTraceDataSizeCode(trace.typeSize);
        _variables[_amountOfVariableTraces] = trace;
        _traceTableLookup[trace.id] = &_variables[_amountOfVariableTraces];
        _amountOfVariableTraces++;
    }

    template <typename T>
    void TraceController::addArray(const char* name, T* ref, uint32_t length) {
        if (_amountOfTraces >= maxTracedVariables) {
            return;
        }
        ArrayTrace trace;
        trace.name = name;
        trace.id = _amountOfTraces++;
        trace.length = length;
        trace.value = reinterpret_cast<uint8_t*>(ref);
        _arrays[_amountOfArrayTraces] = trace;
        _traceTableLookup[trace.id] = &_arrays[_amountOfArrayTraces];
        _amountOfArrayTraces++;
    }
} // namespace IntegralMotions::Tracing