#include <gtest/gtest.h>
#include "trace_communication/buffer_trace_communication.h"
#include "fast_copy.h"

class TestBufferTraceCommunication: public BufferTraceCommunication {
public:
	std::vector<uint8_t> output;
	std::vector<uint8_t> input;
	size_t inputIndex = 0;
public:
	TestBufferTraceCommunication(size_t bufferSize) :
			BufferTraceCommunication(bufferSize) {
	}
	void flush() override {
		_writeBufferIndex = 0;
	}
	virtual bool tryRead(uint8_t *data, const uint8_t data_size) override
	{
		if (inputIndex + data_size <= input.size()) {
			memcpy(data, input.data() + inputIndex, data_size);
			inputIndex += data_size;
			return true;
		}
		return false;
	}
	uint8_t* getBuffer() {
		return _writeBuffer;
	}

public:
	void injectInput(uint8_t *data, uint8_t length) {
		if (_writeBufferIndex + length > BUFFER_SIZE) {
			return;
		}
		fast_copy(_writeBuffer + _writeBufferIndex, data, length);
		_writeBufferIndex += length;
	}
	void clearOutput() {
		output.clear();
	}
	void reset() {
		input.clear();
		inputIndex = 0;
	}
};

TEST(BufferTraceCommunicationTest, WriteWithinBufferLimits) {
	TestBufferTraceCommunication communication(10);
	uint8_t data[] = { 1, 2, 3, 4, 5 };
	communication.injectInput(data, 5);
	EXPECT_EQ(communication.getBufferIndex(), 5);
	EXPECT_EQ(memcmp(communication.getBuffer(), data, 5), 0);
}

TEST(BufferTraceCommunicationTest, WriteExceedingBufferLimit) {
	TestBufferTraceCommunication communication(5);
	uint8_t data[] = { 1, 2, 3, 4, 5, 6 };
	communication.injectInput(data, 6);
	EXPECT_EQ(communication.getBufferIndex(), 0);
}

TEST(BufferTraceCommunicationTest, FlushResetsBufferIndex) {
	TestBufferTraceCommunication communication(10);
	uint8_t data[] = { 1, 2, 3 };
	communication.injectInput(data, 3);
	communication.flush();
	EXPECT_EQ(communication.getBufferIndex(), 0);
}
