#include <gtest/gtest.h>
#include "trace_header.h"
#include "trace_types.h"
#include "trace_controller.h"
#include "trace_communication/trace_communication.h"
#include <IntegralCommunication/Encoding/SevenBitEncoding.h>

// TestCommunication: buffers writes and simulates reads.
class TestCommunication: public TraceCommunication {
public:
	std::vector<uint8_t> output;
	std::vector<uint8_t> input;
	size_t inputIndex = 0;
protected:
	void write(uint8_t *data, const uint8_t data_size) override {
		output.insert(output.end(), data, data + data_size);
	}
	bool tryRead(uint8_t *data, const uint8_t data_size) override {
		if (inputIndex + data_size <= input.size()) {
			memcpy(data, input.data() + inputIndex, data_size);
			inputIndex += data_size;
			return true;
		}
		return false;
	}
public:
	void injectInput(const std::vector<uint8_t> &data) {
		input.insert(input.end(), data.begin(), data.end());
	}
	void clearOutput() {
		output.clear();
	}
	void reset() {
		input.clear();
		inputIndex = 0;
	}
};

void initializeTracer(TraceController &controller,
		TestCommunication &communication) {
	TraceHeader header =
			{ 0, TraceMessageType::CONFIG, TraceValueType::UNKNOWN };

	uint8_t bytes[2] = { 0 };
	getBytesFromTraceHeader(header, bytes);

	std::vector<uint8_t> data;
	data.push_back(bytes[0]);
	data.push_back(bytes[1]);

	communication.injectInput(data);

	controller.loop();
	controller.loop();
	controller.loop();
	controller.loop();
}

TEST(TraceControllerTest, DontSendWithoutConfig) {
	TestCommunication communication;
	TraceController::init(&communication);
	auto &controller = TraceController::get();

	uint8_t variable = 69;
	controller.addVariable("Variable", &variable);

	controller.loop();
	controller.loop();
	controller.loop();

	EXPECT_EQ(communication.output.size(), 0);
}

TEST(TraceControllerTest, SendAfterReceiveConfig) {
	TestCommunication communication;
	TraceController::init(&communication);
	auto &controller = TraceController::get();

	uint8_t variable = 69;
	controller.addVariable("Variable", &variable);

	initializeTracer(controller, communication);

	TraceHeader configHeader = { 0, TraceMessageType::CONFIG,
			TraceValueType::UINT8 };
	TraceHeader valueHeader = { 0, TraceMessageType::UPDATE,
			TraceValueType::UINT8 };

	uint8_t configHeaderbytes[2] = { 0 };
	getBytesFromTraceHeader(configHeader, configHeaderbytes);

	uint8_t valueHeaderbytes[2] = { 0 };
	getBytesFromTraceHeader(valueHeader, valueHeaderbytes);

	uint8_t expected[17];

	uint8_t msg1[] = { configHeaderbytes[0], configHeaderbytes[1], 0x56, 0x61,
			0x72, 0x69, 0x61, 0x62, 0x6c, 0x65, 0x00 };
	uint8_t msg2[] = { valueHeaderbytes[0], valueHeaderbytes[1], 69 };

	SevenBitEncoding::encodeBuffer(msg1, 11, expected);
	SevenBitEncoding::encodeBuffer(msg2, 3, &expected[13]);

	uint8_t values[17];
	std::copy(communication.output.begin(), communication.output.end(), values);

	for (uint8_t i = 0; i < 17; i++) {
		SCOPED_TRACE(testing::Message() << "Index i=" << static_cast<int>(i)
						<< " value=" << static_cast<int>(values[i])
						<< " expected=" << static_cast<int>(expected[i]));
		EXPECT_EQ(values[i], expected[i]);
	}
}
