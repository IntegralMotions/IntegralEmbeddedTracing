#include <IntegralCommunication/CRC.h>
#include <IntegralCommunication/CobsEncodedCommunication.h>
#include <IntegralCommunication/Encoding/SevenBitEncoding.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <span>

#include "TraceController.h"
#include "TraceDataHeader.h"
#include "TraceTypes.h"

namespace {
    constexpr uint16_t LowByteMask = 0x00FF;
    constexpr uint8_t BitsPerByte = 8;
    constexpr size_t StartTraceVariableCountOffset = 1;
    constexpr size_t StartTraceVariableIdsOffset = 2;
    constexpr size_t MaxIncomingMessageSize = 1024;
    constexpr size_t MaxStartTraceResponseSize = 1027;
    constexpr size_t MaxTraceDataResponseSize = 1024;
    constexpr size_t FramedCommunicationTxSize = 4096;
    constexpr size_t FramedCommunicationRxSize = 4096;
    constexpr size_t StartTraceMappingEntrySize = 2;
    constexpr size_t StartTraceErrorEntrySize = 2;
    constexpr size_t CrcSize = 2;
    constexpr size_t TraceDataHeaderSize = sizeof(IntegralMotions::Tracing::TraceDataHeader);

    enum class StartTraceErrorCode : uint8_t {
        VariableNotFound = 0x01,
        TooManyVariables = 0x02,
        DuplicateVariable = 0x03,
    };

} // namespace

namespace IntegralMotions::Tracing {

    TraceController* TraceController::instance = nullptr;

    void TraceController::init(Communication& comm) {
        if (instance == nullptr) {
            instance = new TraceController(comm);
        }
    }

    TraceController& TraceController::get() {
        return *instance;
    }

    TraceController::TraceController(Communication& comm)
        : _framedCommunication(comm, FramedCommunicationTxSize, FramedCommunicationRxSize) {}

    void TraceController::loop() {
        checkForMessage();
        checkForUpdates();
    }

    void TraceController::checkForMessage() {
        std::array<uint8_t, MaxIncomingMessageSize> messageBuffer{};
        size_t messageSize = 0;
        if (!_framedCommunication.readMessage(messageBuffer.data(), messageBuffer.size(), messageSize)) {
            return;
        }

        if (messageSize < 3) {
            return;
        }

        const size_t CrcIndex = messageSize - CrcSize;
        const uint16_t Crc = static_cast<uint16_t>(messageBuffer[CrcIndex]) |
                             (static_cast<uint16_t>(messageBuffer[CrcIndex + 1]) << BitsPerByte);

        Message message{.buffer = {messageBuffer.data(), CrcIndex}, .crc = Crc};
        handleIncomingMessage(message);
    }

    void TraceController::handleIncomingMessage(const Message& message) {
        if (message.buffer.empty()) {
            return;
        }

        switch (static_cast<TraceProtocolMessageType>(message.buffer[0])) {
        case TraceProtocolMessageType::GetConfigRequest:
            sendGetConfigResponse(message);
            break;
        case TraceProtocolMessageType::GetArrayConfigRequest:
            sendGetArrayConfigResponse(message);
            break;
        case TraceProtocolMessageType::StartTraceRequest:
            sendStartTraceResponse(message);
            break;
        case TraceProtocolMessageType::StopTraceEvent:
            _tracingStarted = false;
            break;
        default:
            break;
        }
    }

    void TraceController::checkForUpdates() {
        if (!_tracingStarted) {
            return;
        }

        std::array<uint8_t, MaxTraceDataResponseSize> response{};
        size_t responseIndex = 0;
        uint8_t traceDataLength = 0;

        response[responseIndex++] = static_cast<uint8_t>(TraceProtocolMessageType::TraceData);
        const size_t TraceDataLengthIndex = responseIndex++;

        for (uint8_t i = 0; i < _amountOfTracedVariables; i++) {
            Trace* trace = _traceTableLookup[_tracedVariableIds[i]];
            if (trace == nullptr || trace->value == nullptr || trace->previousValue == nullptr) {
                continue;
            }

            if (memcmp(trace->value, trace->previousValue, trace->typeSize) != 0) {
                const size_t TraceDataEntrySize = TraceDataHeaderSize + trace->typeSize;
                if ((responseIndex + TraceDataEntrySize + CrcSize) > response.size()) {
                    break;
                }

                TraceDataHeader traceDataHeader{.variableId = i, .sizeCode = trace->updateHeader.sizeCode};
                memcpy(response.data() + responseIndex, &traceDataHeader, TraceDataHeaderSize);
                responseIndex += TraceDataHeaderSize;
                memcpy(response.data() + responseIndex, trace->value, trace->typeSize);
                responseIndex += trace->typeSize;
                memcpy(trace->previousValue, trace->value, trace->typeSize);
                traceDataLength++;
            }
        }

        if (traceDataLength == 0) {
            return;
        }

        response[TraceDataLengthIndex] = traceDataLength;

        const uint16_t Crc = CRC::calculate(response.data(), responseIndex);
        response[responseIndex++] = static_cast<uint8_t>(Crc & LowByteMask);
        response[responseIndex++] = static_cast<uint8_t>((Crc >> BitsPerByte) & LowByteMask);

        _framedCommunication.writeMessage(response.data(), responseIndex);
    }

    void TraceController::sendGetConfigResponse(const Message& message) {
        constexpr size_t MaxResponseSize = 1024;
        std::array<uint8_t, MaxResponseSize> response{};

        size_t responseIndex = 0;
        uint8_t variableCount = 0;

        response[responseIndex++] = static_cast<uint8_t>(TraceProtocolMessageType::GetConfigResponse);
        const size_t VariableCountIndex = responseIndex++;

        for (uint16_t i = 0; i < _amountOfVariableTraces; i++) {
            const VariableTrace& trace = _variables[i];
            const size_t NameLength = std::min(strlen(trace.name), static_cast<size_t>(UINT8_MAX));

            const size_t VariableEntrySize = 3 + NameLength;
            if ((responseIndex + VariableEntrySize + 2) > response.size()) {
                break;
            }

            response[responseIndex++] = static_cast<uint8_t>(trace.id);
            response[responseIndex++] = static_cast<uint8_t>(trace.type);
            response[responseIndex++] = static_cast<uint8_t>(NameLength);
            memcpy(response.data() + responseIndex, trace.name, NameLength);
            responseIndex += NameLength;
            variableCount++;
        }

        response[VariableCountIndex] = variableCount;

        const uint16_t Crc = CRC::calculate(response.data(), responseIndex);
        response[responseIndex++] = static_cast<uint8_t>(Crc & LowByteMask);
        response[responseIndex++] = static_cast<uint8_t>((Crc >> BitsPerByte) & LowByteMask);

        _framedCommunication.writeMessage(response.data(), responseIndex);
        _variableConfigSend = true;
    }

    void TraceController::sendGetArrayConfigResponse(const Message& message) {
        constexpr size_t MaxResponseSize = 1024;
        std::array<uint8_t, MaxResponseSize> response{};

        size_t responseIndex = 0;
        uint8_t arrayVariableCount = 0;

        response[responseIndex++] = static_cast<uint8_t>(TraceProtocolMessageType::GetArrayConfigResponse);
        const size_t ArrayVariableCountIndex = responseIndex++;

        for (uint16_t i = 0; i < _amountOfArrayTraces; i++) {
            const ArrayTrace& trace = _arrays[i];
            const size_t NameLength = std::min(strlen(trace.name), static_cast<size_t>(UINT8_MAX));

            const size_t ArrayVariableEntrySize = 5 + NameLength;
            if ((responseIndex + ArrayVariableEntrySize + 2) > response.size()) {
                break;
            }

            const uint16_t ArrayLength = std::min(trace.length, static_cast<uint32_t>(UINT16_MAX));
            response[responseIndex++] = static_cast<uint8_t>(trace.id);
            response[responseIndex++] = static_cast<uint8_t>(trace.type);
            response[responseIndex++] = static_cast<uint8_t>(ArrayLength & LowByteMask);
            response[responseIndex++] = static_cast<uint8_t>((ArrayLength >> BitsPerByte) & LowByteMask);
            response[responseIndex++] = static_cast<uint8_t>(NameLength);
            memcpy(response.data() + responseIndex, trace.name, NameLength);
            responseIndex += NameLength;
            arrayVariableCount++;
        }

        response[ArrayVariableCountIndex] = arrayVariableCount;

        const uint16_t Crc = CRC::calculate(response.data(), responseIndex);
        response[responseIndex++] = static_cast<uint8_t>(Crc & LowByteMask);
        response[responseIndex++] = static_cast<uint8_t>((Crc >> BitsPerByte) & LowByteMask);

        _framedCommunication.writeMessage(response.data(), responseIndex);
        _arrayConfigSend = true;
    }

    void TraceController::sendStartTraceResponse(const Message& message) {
        if (message.buffer.size() <= StartTraceVariableCountOffset) {
            return;
        }

        std::array<uint8_t, MaxStartTraceResponseSize> response{};
        size_t responseIndex = 0;
        response[responseIndex++] = static_cast<uint8_t>(TraceProtocolMessageType::StartTraceResponse);

        _amountOfTracedVariables = 0;
        uint8_t mappingCount = 0;
        const size_t MappingCountIndex = responseIndex++;

        const size_t RequestedVariableCount = message.buffer[StartTraceVariableCountOffset];
        const size_t ErrorCountIndex = responseIndex + (RequestedVariableCount * StartTraceMappingEntrySize);
        size_t errorResponseIndex = ErrorCountIndex + 1;
        uint8_t errorCount = 0;

        for (size_t i = 0; i < RequestedVariableCount; i++) {
            const size_t VariableIdIndex = StartTraceVariableIdsOffset + i;
            if (VariableIdIndex >= message.buffer.size()) {
                break;
            }

            const uint8_t VariableId = message.buffer[VariableIdIndex];
            StartTraceErrorCode errorCode = StartTraceErrorCode::VariableNotFound;
            bool hasError = false;

            if (_traceTableLookup[VariableId] == nullptr) {
                hasError = true;
                errorCode = StartTraceErrorCode::VariableNotFound;
            }

            for (uint8_t j = 0; j < _amountOfTracedVariables; j++) {
                if (_tracedVariableIds[j] == VariableId) {
                    hasError = true;
                    errorCode = StartTraceErrorCode::DuplicateVariable;
                    break;
                }
            }

            if (!hasError && _amountOfTracedVariables >= TraceController::MaxActiveTraceVariables) {
                hasError = true;
                errorCode = StartTraceErrorCode::TooManyVariables;
            }

            if (hasError) {
                if ((errorResponseIndex + StartTraceErrorEntrySize + CrcSize) > response.size()) {
                    break;
                }
                response[errorResponseIndex++] = VariableId;
                response[errorResponseIndex++] = static_cast<uint8_t>(errorCode);
                errorCount++;
                continue;
            }

            if ((responseIndex + StartTraceMappingEntrySize) > ErrorCountIndex) {
                break;
            }

            const uint8_t TraceVariableId = _amountOfTracedVariables;
            _tracedVariableIds[_amountOfTracedVariables++] = VariableId;
            response[responseIndex++] = VariableId;
            response[responseIndex++] = TraceVariableId;
            mappingCount++;
        }

        response[MappingCountIndex] = mappingCount;
        response[responseIndex++] = errorCount;

        if (errorCount > 0) {
            memmove(response.data() + responseIndex, response.data() + ErrorCountIndex + 1,
                    errorCount * StartTraceErrorEntrySize);
            responseIndex += errorCount * StartTraceErrorEntrySize;
        }

        const uint16_t Crc = CRC::calculate(response.data(), responseIndex);
        response[responseIndex++] = static_cast<uint8_t>(Crc & LowByteMask);
        response[responseIndex++] = static_cast<uint8_t>((Crc >> BitsPerByte) & LowByteMask);

        _framedCommunication.writeMessage(response.data(), responseIndex);
        _tracingStarted = mappingCount > 0;
    }
} // namespace IntegralMotions::Tracing