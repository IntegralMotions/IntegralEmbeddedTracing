#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include <IntegralCommunication/CRC.h>
#include <IntegralCommunication/Communication.h>
#include <IntegralCommunication/Encoding/CobsEncoding.h>

#include "TraceController.h"
#include "TraceDataHeader.h"
#include "TraceTypes.h"

using namespace IntegralMotions::Tracing;

class TestCommunication : public Communication {
  public:
    std::vector<uint8_t> input;
    std::vector<uint8_t> output;

  private:
    size_t writeImpl(const uint8_t* data, size_t size) override {
        output.insert(output.end(), data, data + size);
        return size;
    }

    size_t availableImpl() override {
        return input.size();
    }

    size_t readImpl(uint8_t* data, size_t size) override {
        const size_t readSize = std::min(size, input.size());
        std::copy(input.begin(), input.begin() + readSize, data);
        input.erase(input.begin(), input.begin() + readSize);
        return readSize;
    }
};

namespace {
    constexpr uint16_t LowByteMask = 0x00FF;
    constexpr uint8_t BitsPerByte = 8;
    constexpr uint8_t VariableNotFoundError = 0x01;
    constexpr uint8_t DuplicateVariableError = 0x03;
    constexpr uint8_t FirstVariableId = 0;
    constexpr uint8_t SecondVariableId = 1;
    constexpr uint8_t MissingVariableId = 200;

    void appendCrc(std::vector<uint8_t>& payload) {
        const uint16_t crc = CRC::calculate(payload.data(), payload.size());
        payload.push_back(static_cast<uint8_t>(crc & LowByteMask));
        payload.push_back(static_cast<uint8_t>((crc >> BitsPerByte) & LowByteMask));
    }

    std::vector<uint8_t> encodeFrame(std::vector<uint8_t> payload) {
        appendCrc(payload);

        std::vector<uint8_t> encoded(CobsEncoding::getEncodedBufferSize(payload.size()) + 1U);
        const size_t encodedSize = CobsEncoding::encodeBuffer(payload.data(), payload.size(), encoded.data());
        encoded[encodedSize] = CobsEncoding::Delimiter;
        encoded.resize(encodedSize + 1U);
        return encoded;
    }

    std::vector<std::vector<uint8_t>> decodeFrames(const std::vector<uint8_t>& frames) {
        std::vector<std::vector<uint8_t>> decodedFrames;
        size_t frameStart = 0;

        while (frameStart < frames.size()) {
            const auto delimiter = std::find(frames.begin() + static_cast<std::ptrdiff_t>(frameStart), frames.end(),
                                             CobsEncoding::Delimiter);
            if (delimiter == frames.end()) {
                break;
            }

            const size_t frameEnd = static_cast<size_t>(std::distance(frames.begin(), delimiter));
            std::vector<uint8_t> decoded(256);
            size_t decodedSize = 0;
            const bool success = CobsEncoding::decodeBuffer(frames.data() + frameStart, frameEnd - frameStart,
                                                            decoded.data(), decoded.size(), decodedSize);
            EXPECT_TRUE(success);
            decoded.resize(decodedSize);

            if (decoded.size() < 2U) {
                ADD_FAILURE() << "Decoded frame is too small to contain a CRC";
                return decodedFrames;
            }
            const size_t crcIndex = decoded.size() - 2U;
            const uint16_t crc = static_cast<uint16_t>(decoded[crcIndex]) |
                                 (static_cast<uint16_t>(decoded[crcIndex + 1U]) << BitsPerByte);
            EXPECT_TRUE(CRC::validate(decoded.data(), crcIndex, crc));
            decoded.resize(crcIndex);
            decodedFrames.push_back(decoded);

            frameStart = frameEnd + 1U;
        }

        return decodedFrames;
    }

    std::vector<uint8_t> takeSingleResponse(TestCommunication& communication) {
        const std::vector<std::vector<uint8_t>> responses = decodeFrames(communication.output);
        communication.output.clear();
        if (responses.size() != 1U) {
            ADD_FAILURE() << "Expected exactly one response frame, got " << responses.size();
            return {};
        }
        return responses[0];
    }

    void sendMessage(TestCommunication& communication, const std::vector<uint8_t>& payload) {
        communication.input = encodeFrame(payload);
    }

    TraceDataHeader readTraceDataHeader(const std::vector<uint8_t>& response, size_t offset) {
        TraceDataHeader header{};
        memcpy(&header, response.data() + offset, sizeof(header));
        return header;
    }
} // namespace

TEST(TraceControllerTest, FullControllerFlowCoversConfigStartUpdatesAndStop) {
    TestCommunication communication;
    TraceController::init(communication);
    TraceController& controller = TraceController::get();

    uint8_t firstValue = 42;
    int16_t secondValue = -300;

    {
        SCOPED_TRACE("addVariable and loop send variable config");
        controller.addVariable("first", &firstValue);
        controller.addVariable("second", &secondValue);

        sendMessage(communication, {static_cast<uint8_t>(TraceProtocolMessageType::GetConfigRequest)});
        controller.loop();

        const std::vector<uint8_t> Response = takeSingleResponse(communication);
        ASSERT_EQ(Response.size(), 19U);
        EXPECT_EQ(Response[0], static_cast<uint8_t>(TraceProtocolMessageType::GetConfigResponse));
        EXPECT_EQ(Response[1], 2U);

        EXPECT_EQ(Response[2], FirstVariableId);
        EXPECT_EQ(Response[3], static_cast<uint8_t>(TraceValueType::UInt8));
        EXPECT_EQ(Response[4], 5U);
        EXPECT_EQ(Response[5], static_cast<uint8_t>('f'));
        EXPECT_EQ(Response[6], static_cast<uint8_t>('i'));
        EXPECT_EQ(Response[7], static_cast<uint8_t>('r'));
        EXPECT_EQ(Response[8], static_cast<uint8_t>('s'));
        EXPECT_EQ(Response[9], static_cast<uint8_t>('t'));

        EXPECT_EQ(Response[10], SecondVariableId);
        EXPECT_EQ(Response[11], static_cast<uint8_t>(TraceValueType::Int16));
        EXPECT_EQ(Response[12], 6U);
        EXPECT_EQ(Response[13], static_cast<uint8_t>('s'));
        EXPECT_EQ(Response[14], static_cast<uint8_t>('e'));
        EXPECT_EQ(Response[15], static_cast<uint8_t>('c'));
        EXPECT_EQ(Response[16], static_cast<uint8_t>('o'));
        EXPECT_EQ(Response[17], static_cast<uint8_t>('n'));
        EXPECT_EQ(Response[18], static_cast<uint8_t>('d'));
    }

    {
        SCOPED_TRACE("start trace maps valid variables and reports duplicate and missing ids");
        sendMessage(communication, {static_cast<uint8_t>(TraceProtocolMessageType::StartTraceRequest), 4U,
                                    FirstVariableId, SecondVariableId, FirstVariableId, MissingVariableId});
        controller.checkForMessage();

        const std::vector<uint8_t> response = takeSingleResponse(communication);
        ASSERT_EQ(response.size(), 11U);
        EXPECT_EQ(response[0], static_cast<uint8_t>(TraceProtocolMessageType::StartTraceResponse));
        EXPECT_EQ(response[1], 2U);
        EXPECT_EQ(response[2], FirstVariableId);
        EXPECT_EQ(response[3], 0U);
        EXPECT_EQ(response[4], SecondVariableId);
        EXPECT_EQ(response[5], 1U);
        EXPECT_EQ(response[6], 2U);
        EXPECT_EQ(response[7], FirstVariableId);
        EXPECT_EQ(response[8], DuplicateVariableError);
        EXPECT_EQ(response[9], MissingVariableId);
        EXPECT_EQ(response[10], VariableNotFoundError);
    }

    {
        SCOPED_TRACE("first checkForUpdates sends all traced variables");
        controller.checkForUpdates();

        const std::vector<uint8_t> response = takeSingleResponse(communication);
        ASSERT_EQ(response.size(), 7U);
        EXPECT_EQ(response[0], static_cast<uint8_t>(TraceProtocolMessageType::TraceData));
        EXPECT_EQ(response[1], 2U);

        TraceDataHeader firstHeader = readTraceDataHeader(response, 2U);
        EXPECT_EQ(firstHeader.variableId, 0U);
        EXPECT_EQ(firstHeader.sizeCode, TraceDataSizeCode::OneByte);
        EXPECT_EQ(response[3], firstValue);

        TraceDataHeader secondHeader = readTraceDataHeader(response, 4U);
        EXPECT_EQ(secondHeader.variableId, 1U);
        EXPECT_EQ(secondHeader.sizeCode, TraceDataSizeCode::TwoBytes);
        int16_t tracedSecondValue = 0;
        memcpy(&tracedSecondValue, response.data() + 5U, sizeof(tracedSecondValue));
        EXPECT_EQ(tracedSecondValue, secondValue);
    }

    {
        SCOPED_TRACE("same values do not send trace data");
        controller.checkForUpdates();
        EXPECT_TRUE(communication.output.empty());
    }

    {
        SCOPED_TRACE("changing one value sends only that mapped trace variable");
        firstValue = 43;
        controller.checkForUpdates();

        const std::vector<uint8_t> response = takeSingleResponse(communication);
        ASSERT_EQ(response.size(), 4U);
        EXPECT_EQ(response[0], static_cast<uint8_t>(TraceProtocolMessageType::TraceData));
        EXPECT_EQ(response[1], 1U);
        TraceDataHeader header = readTraceDataHeader(response, 2U);
        EXPECT_EQ(header.variableId, 0U);
        EXPECT_EQ(header.sizeCode, TraceDataSizeCode::OneByte);
        EXPECT_EQ(response[3], firstValue);
    }

    {
        SCOPED_TRACE("assigning the same value again does not send trace data");
        firstValue = 43;
        controller.checkForUpdates();
        EXPECT_TRUE(communication.output.empty());
    }

    {
        SCOPED_TRACE("changing the second value sends the second mapped trace variable");
        secondValue = 1234;
        controller.checkForUpdates();

        const std::vector<uint8_t> response = takeSingleResponse(communication);
        ASSERT_EQ(response.size(), 5U);
        EXPECT_EQ(response[0], static_cast<uint8_t>(TraceProtocolMessageType::TraceData));
        EXPECT_EQ(response[1], 1U);
        TraceDataHeader header = readTraceDataHeader(response, 2U);
        EXPECT_EQ(header.variableId, 1U);
        EXPECT_EQ(header.sizeCode, TraceDataSizeCode::TwoBytes);
        int16_t tracedSecondValue = 0;
        memcpy(&tracedSecondValue, response.data() + 3U, sizeof(tracedSecondValue));
        EXPECT_EQ(tracedSecondValue, secondValue);
    }

    {
        SCOPED_TRACE("stop trace has no response and stops later updates");
        sendMessage(communication, {static_cast<uint8_t>(TraceProtocolMessageType::StopTraceEvent)});
        controller.checkForMessage();
        EXPECT_TRUE(communication.output.empty());

        firstValue = 44;
        secondValue = 4321;
        controller.checkForUpdates();
        EXPECT_TRUE(communication.output.empty());
    }

    {
        SCOPED_TRACE("start trace with only invalid ids does not start tracing");
        sendMessage(communication,
                    {static_cast<uint8_t>(TraceProtocolMessageType::StartTraceRequest), 1U, MissingVariableId});
        controller.checkForMessage();

        const std::vector<uint8_t> response = takeSingleResponse(communication);
        ASSERT_EQ(response.size(), 5U);
        EXPECT_EQ(response[0], static_cast<uint8_t>(TraceProtocolMessageType::StartTraceResponse));
        EXPECT_EQ(response[1], 0U);
        EXPECT_EQ(response[2], 1U);
        EXPECT_EQ(response[3], MissingVariableId);
        EXPECT_EQ(response[4], VariableNotFoundError);

        firstValue = 45;
        controller.checkForUpdates();
        EXPECT_TRUE(communication.output.empty());
    }

    {
        SCOPED_TRACE("malformed start request does not send a response");
        sendMessage(communication, {static_cast<uint8_t>(TraceProtocolMessageType::StartTraceRequest)});
        controller.checkForMessage();
        EXPECT_TRUE(communication.output.empty());
    }
}
