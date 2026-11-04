#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include <Encoding/CobsEncoding.h>

#include "TraceController.h"
#include "TraceControllerTestGuard.h"
#include "TraceTypes.h"

namespace {
    class TestLogger : public IntegralMotions::Diagnostics::Logger {
      public:
        std::vector<std::string> messages;

        void log(IntegralMotions::Diagnostics::LogLevel level, std::string_view format,
                 std::span<const IntegralMotions::Diagnostics::LogValue>) override {
            EXPECT_EQ(level, IntegralMotions::Diagnostics::LogLevel::Warning);
            messages.emplace_back(format);
        }
    };

    class TestCommunication : public Communication {
      public:
        std::vector<uint8_t> input;

      private:
        size_t writeImpl(const uint8_t*, size_t size) override { return size; }
        size_t availableImpl() override { return input.size(); }
        size_t readImpl(uint8_t* data, size_t size) override {
            const size_t count = std::min(size, input.size());
            std::copy_n(input.begin(), count, data);
            input.erase(input.begin(), input.begin() + count);
            return count;
        }
    };

    void sendFrame(TestCommunication& communication, const std::vector<uint8_t>& payload) {
        communication.input.resize(IntegralMotions::Encoding::CobsEncoding::getEncodedBufferSize(payload.size()) + 1);
        const size_t size = IntegralMotions::Encoding::CobsEncoding::encodeBuffer(
            payload.data(), payload.size(), communication.input.data());
        communication.input[size] = IntegralMotions::Encoding::CobsEncoding::Delimiter;
        communication.input.resize(size + 1);
    }
} // namespace

TEST(TraceControllerLoggingTest, ForwardsCobsWarningsToApplicationLogger) {
    TestCommunication communication;
    TestLogger logger;
    TestCommunication secondCommunication;
    TestLogger secondLogger;
    TraceControllerTestGuard guard;
    IntegralMotions::Tracing::TraceController::init(communication, logger);
    auto& controller = IntegralMotions::Tracing::TraceController::get();

    controller.checkForMessage();
    EXPECT_TRUE(logger.messages.empty());

    struct UnsupportedType { int value; } unsupported{};
    controller.addVariable("unsupported", &unsupported);
    ASSERT_EQ(logger.messages.size(), 1U);
    EXPECT_NE(logger.messages.back().find("unsupported type"), std::string::npos);

    sendFrame(communication, {1});
    controller.checkForMessage();
    ASSERT_EQ(logger.messages.size(), 2U);
    EXPECT_NE(logger.messages.back().find("message too short"), std::string::npos);

    sendFrame(communication, {0xff, 0, 0});
    controller.checkForMessage();
    ASSERT_EQ(logger.messages.size(), 3U);
    EXPECT_NE(logger.messages.back().find("Unknown trace message type"), std::string::npos);

    const auto start = static_cast<uint8_t>(IntegralMotions::Tracing::TraceProtocolMessageType::StartTraceRequest);
    sendFrame(communication, {start, 0, 0});
    controller.checkForMessage();
    ASSERT_EQ(logger.messages.size(), 4U);
    EXPECT_NE(logger.messages.back().find("missing variable count"), std::string::npos);

    sendFrame(communication, {start, 2, 0, 0});
    controller.checkForMessage();
    ASSERT_EQ(logger.messages.size(), 5U);
    EXPECT_NE(logger.messages.back().find("request truncated"), std::string::npos);

    communication.input.assign(4096, 1); // Fill the receive buffer without a frame delimiter.
    controller.checkForMessage();
    controller.checkForMessage();
    ASSERT_EQ(logger.messages.size(), 6U);
    EXPECT_NE(logger.messages.back().find("COBS receive buffer full"), std::string::npos);

    IntegralMotions::Tracing::TraceController::shutdown();
    IntegralMotions::Tracing::TraceController::shutdown(); // Safe when already shut down.

    IntegralMotions::Tracing::TraceController::init(secondCommunication, secondLogger);
    auto& secondController = IntegralMotions::Tracing::TraceController::get();
    uint8_t value = 0;
    secondController.addVariable("value", &value);
    sendFrame(secondCommunication, {1});
    secondController.checkForMessage();
    ASSERT_EQ(secondLogger.messages.size(), 1U);
    EXPECT_NE(secondLogger.messages[0].find("message too short"), std::string::npos);
    EXPECT_EQ(logger.messages.size(), 6U);
    IntegralMotions::Tracing::TraceController::shutdown();
}
