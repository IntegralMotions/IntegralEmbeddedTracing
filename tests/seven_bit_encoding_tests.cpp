#include <gtest/gtest.h>
#include <vector>
#include <random>
#include "seven_bit_encoding.h"

TEST(SevenBitEncoding, LeftMaskTest) {
	EXPECT_EQ(SevenBitEncoding::leftMask(0), static_cast<uint8_t>(0x00));
	EXPECT_EQ(SevenBitEncoding::leftMask(1), static_cast<uint8_t>(0x01));
	EXPECT_EQ(SevenBitEncoding::leftMask(2), static_cast<uint8_t>(0x03));
	EXPECT_EQ(SevenBitEncoding::leftMask(3), static_cast<uint8_t>(0x07));
	EXPECT_EQ(SevenBitEncoding::leftMask(4), static_cast<uint8_t>(0x0F));
	EXPECT_EQ(SevenBitEncoding::leftMask(5), static_cast<uint8_t>(0x1F));
	EXPECT_EQ(SevenBitEncoding::leftMask(6), static_cast<uint8_t>(0x3F));
	EXPECT_EQ(SevenBitEncoding::leftMask(7), static_cast<uint8_t>(0x7F));
	EXPECT_EQ(SevenBitEncoding::leftMask(8), static_cast<uint8_t>(0xFF));
}

struct EncodedSizeTestCase {
	uint32_t value;
	size_t expectedSize;
};

class EncodedSizeTest: public ::testing::TestWithParam<EncodedSizeTestCase> {
};

TEST_P(EncodedSizeTest, GetEncodedSize) {
	const auto &tc = GetParam();
	EXPECT_EQ(SevenBitEncoding::getEncodedSize(tc.value), tc.expectedSize);
}

INSTANTIATE_TEST_SUITE_P(SevenBitEncoding, EncodedSizeTest,
		::testing::Values(EncodedSizeTestCase { 0, 1 }, EncodedSizeTestCase { 1,
				1 }, EncodedSizeTestCase { 127, 1 }, EncodedSizeTestCase { 128,
				2 }, EncodedSizeTestCase { 255, 2 }, EncodedSizeTestCase {
				16383, 2 }, EncodedSizeTestCase { 16384, 3 },
				EncodedSizeTestCase { 2097151, 3 }, EncodedSizeTestCase {
						2097152, 4 }, EncodedSizeTestCase { 268435455, 4 },
				EncodedSizeTestCase { 268435456, 5 }, EncodedSizeTestCase {
						4294967295, 5 }));

TEST(SevenBitEncoding, EncodeValue) {
	uint8_t output[5] = { 0 };

	SevenBitEncoding::encodeValue(127, output);
	EXPECT_EQ(output[0], 127);

	SevenBitEncoding::encodeValue(128, output);
	EXPECT_EQ(output[0], 128 | 0x80);
	EXPECT_EQ(output[1], 1);

	SevenBitEncoding::encodeValue(16384, output);
	EXPECT_EQ(output[0], 128 | 0x80);
	EXPECT_EQ(output[1], 128 | 0x80);
	EXPECT_EQ(output[2], 1);
}

TEST(SevenBitEncoding, DecodeValue) {
	size_t consumedBytes;
	std::vector<uint8_t> input = { 127 };
	EXPECT_EQ(
			SevenBitEncoding::decodeValue(input.data(), input.size(),
					consumedBytes), 127);
	EXPECT_EQ(consumedBytes, 1);

	input = { 128 | 0x80, 1 };
	EXPECT_EQ(
			SevenBitEncoding::decodeValue(input.data(), input.size(),
					consumedBytes), 128);
	EXPECT_EQ(consumedBytes, 2);

	input = { 128 | 0x80, 128 | 0x80, 1 };
	EXPECT_EQ(
			SevenBitEncoding::decodeValue(input.data(), input.size(),
					consumedBytes), 16384);
	EXPECT_EQ(consumedBytes, 3);
}

TEST(SevenBitEncoding, DecodeValueInvalidInput) {
	size_t consumedBytes;
	std::vector<uint8_t> input = { 128 | 0x80, 128 | 0x80, 128 | 0x80, 128
			| 0x80, 128 | 0x80 };
	EXPECT_EQ(
			SevenBitEncoding::decodeValue(input.data(), input.size(),
					consumedBytes), 0);
	EXPECT_EQ(consumedBytes, 5);
}

class GetEncodedBufferSizeTest: public ::testing::TestWithParam<
		EncodedSizeTestCase> {
};

TEST_P(GetEncodedBufferSizeTest, ComputesCorrectSize) {
	const auto &tc = GetParam();
	EXPECT_EQ(SevenBitEncoding::getEncodedBufferSize(tc.value),
			tc.expectedSize);
}

INSTANTIATE_TEST_SUITE_P(SevenBitEncoding, GetEncodedBufferSizeTest,
		::testing::Values(EncodedSizeTestCase { 0, 1 }, EncodedSizeTestCase { 1,
				2 }, EncodedSizeTestCase { 2, 3 }, EncodedSizeTestCase { 3, 4 }, EncodedSizeTestCase { 4, 5 },EncodedSizeTestCase { 5, 6 }, EncodedSizeTestCase { 6, 7 },
				EncodedSizeTestCase { 7, 8 }, EncodedSizeTestCase { 8, 10 },EncodedSizeTestCase { 9, 11 }, EncodedSizeTestCase { 10, 12 },
				EncodedSizeTestCase { 11, 13 }, EncodedSizeTestCase { 12, 14 },EncodedSizeTestCase { 13, 15 }, EncodedSizeTestCase { 14, 16 },
				EncodedSizeTestCase { 15, 18 }, EncodedSizeTestCase { 16, 19 },
				EncodedSizeTestCase { 18, 21 },
				EncodedSizeTestCase { 127, 146 },
				EncodedSizeTestCase { 128, 147 }));

struct BufferTestCase {
	std::vector<uint8_t> input;
	std::vector<uint8_t> expectedEncoded;
};

class BufferEncodingTest: public ::testing::TestWithParam<BufferTestCase> {
};

TEST_P(BufferEncodingTest, EncodeBuffer) {
	const auto &tc = GetParam();
	size_t bufSize = SevenBitEncoding::getEncodedBufferSize(tc.input.size());
	uint8_t encoded[bufSize];
	size_t encodedLength = SevenBitEncoding::encodeBuffer(tc.input.data(),
			tc.input.size(), encoded);
	EXPECT_EQ(encodedLength, tc.expectedEncoded.size());
	for (size_t i = 0; i < encodedLength; i++) {
		EXPECT_EQ(encoded[i], tc.expectedEncoded[i]);
	}
}

TEST_P(BufferEncodingTest, DecodeBuffer) {
	const auto &tc = GetParam();
	std::vector<uint8_t> decoded(tc.input.size(), 0);
	size_t decodedLength = SevenBitEncoding::decodeBuffer(
			tc.expectedEncoded.data(), tc.expectedEncoded.size(),
			decoded.data(), decoded.size());
	EXPECT_EQ(decodedLength, tc.input.size());
	EXPECT_EQ(decoded, tc.input);
}

INSTANTIATE_TEST_SUITE_P(SevenBitEncoding, BufferEncodingTest,
		::testing::Values(BufferTestCase { { }, { } }, BufferTestCase {
				{ 0xFF }, { 0xFF, 0x40 } }, BufferTestCase { { 0x12, 0x34 }, {
				0x89, 0x8D, 0x00 } }, BufferTestCase { { 0xFF, 0xFF }, { 0xFF,
				0xFF, 0x60 } }, BufferTestCase { { 0x00, 0x01, 0x80 }, { 0x80,
				0x80, 0xB0, 0x00 } }, BufferTestCase { { 0x00, 0x01, 0x02, 0x03,
				0x04, 0x05, 0x06 }, { 0x80, 0x80, 0xA0, 0xA0, 0x98, 0x90, 0x8A,
				0x06 } }, BufferTestCase { { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
				0x06, 0x07, 0x08, 0x09 }, { 0x80, 0x80, 0xA0, 0xA0, 0x98, 0x90,
				0x8A, 0x86, 0x83, 0xC2, 0x81, 0x10 } }, BufferTestCase { { 0x00,
				0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A }, {
				0x80, 0x80, 0xA0, 0xA0, 0x98, 0x90, 0x8A, 0x86, 0x83, 0xC2,
				0x81, 0x90, 0x50 } }, BufferTestCase { { 0x00, 0x01, 0x02, 0x03,
				0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
				0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13 }, { 0x80, 0x80, 0xA0, 0xA0,
				0x98, 0x90, 0x8A, 0x86, 0x83, 0xC2, 0x81, 0x90, 0xD0, 0xAC,
				0x98, 0x8D, 0x87, 0x83, 0xE2, 0x81, 0x88, 0xC8, 0x26, } },
				BufferTestCase { { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
						0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
						0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
						0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F }, { 0x80,
						0x80, 0xA0, 0xA0, 0x98, 0x90, 0x8A, 0x86, 0x83, 0xC2,
						0x81, 0x90, 0xD0, 0xAC, 0x98, 0x8D, 0x87, 0x83, 0xE2,
						0x81, 0x88, 0xC8, 0xA6, 0x94, 0x8A, 0xC5, 0xC2, 0xF1,
						0xC0, 0xE4, 0xB4, 0x9B, 0x8E, 0x87, 0xA3, 0xE1, 0x78 } }));

TEST(SevenBitEncoding, FuzzEncodeDecode) {
	std::mt19937 rng(std::random_device { }());
	std::uniform_int_distribution<size_t> sizeDist(0, 20);
	std::uniform_int_distribution<int> byteDist(0, 255);
	for (int i = 0; i < 1000; i++) {
		size_t len = sizeDist(rng);
		uint8_t *input = new uint8_t[len];
		for (size_t j = 0; j < len; j++) {
			input[j] = static_cast<uint8_t>(byteDist(rng));
		}
		size_t encodedBufferSize = SevenBitEncoding::getEncodedBufferSize(len);
		uint8_t *encoded = new uint8_t[encodedBufferSize];
		size_t encodedLen = SevenBitEncoding::encodeBuffer(input, len, encoded);
		uint8_t *decoded = new uint8_t[len];
		size_t decodedLen = SevenBitEncoding::decodeBuffer(encoded, encodedLen,
				decoded, len);
		EXPECT_EQ(decodedLen, len);
		EXPECT_EQ(0, memcmp(decoded, input, len));
	}
}


TEST(SevenBitEncoding, IsLastByteTest) {
	EXPECT_EQ(SevenBitEncoding::isLastByte(0x7F), true);
	EXPECT_EQ(SevenBitEncoding::isLastByte(0x80), false);
}

