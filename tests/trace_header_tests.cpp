#include <gtest/gtest.h>
#include "trace_header.h"
#include "trace_types.h"

struct TraceHeaderBytesTestCase {
    TraceHeader header;
    uint8_t bytes[2];
};

class TraceHeaderToBytesTest : public ::testing::TestWithParam<TraceHeaderBytesTestCase> { };

TEST_P(TraceHeaderToBytesTest, ConvertsHeaderToBytes) {
    auto testCase = GetParam();
    uint8_t bytes[2] = {0};
    getBytesFromTraceHeader(testCase.header, bytes);
    EXPECT_EQ(bytes[0], testCase.bytes[0]);
    EXPECT_EQ(bytes[1], testCase.bytes[1]);
}

INSTANTIATE_TEST_SUITE_P(
    ToBytesTests,
    TraceHeaderToBytesTest,
    ::testing::Values(
    		TraceHeaderBytesTestCase{ {202, TraceMessageType::CONFIG, TraceValueType::UNKNOWN}, {0b0000'1100, 0b1010'0000} },
			TraceHeaderBytesTestCase{ {2, TraceMessageType::ARRAY_CONFIG, TraceValueType::UNKNOWN}, {0b0100'0000, 0b0010'0000} },
			TraceHeaderBytesTestCase{ {828, TraceMessageType::UPDATE, TraceValueType::UINT8}, {0b1011'0011, 0b1100'0100} },
			TraceHeaderBytesTestCase{ {959, TraceMessageType::UPDATE, TraceValueType::INT32}, {0b1011'1011, 0b1111'0111} },
			TraceHeaderBytesTestCase{ {33, TraceMessageType::UPDATE, TraceValueType::FLOAT}, {0b1000'0010, 0b0001'1011} },
			TraceHeaderBytesTestCase{ {386, TraceMessageType::ARRAY_UPDATE, TraceValueType::INT8}, {0b1101'1000, 0b0010'0011} },
			TraceHeaderBytesTestCase{ {149, TraceMessageType::ARRAY_UPDATE, TraceValueType::UINT16}, {0b1100'1001, 0b0101'0110} },
			TraceHeaderBytesTestCase{ {445, TraceMessageType::ARRAY_UPDATE, TraceValueType::DOUBLE}, {0b1101'1011, 0b1101'1100} }
    )
);

class TraceHeaderFromBytesTest : public ::testing::TestWithParam<TraceHeaderBytesTestCase> { };

TEST_P(TraceHeaderFromBytesTest, ConvertsBytesToHeader) {
    auto testCase = GetParam();
    TraceHeader header = getTraceHeaderFromBytes(testCase.bytes);
    EXPECT_EQ(header.id, testCase.header.id);
    EXPECT_EQ(static_cast<int>(header.messageType), static_cast<int>(testCase.header.messageType));
    EXPECT_EQ(static_cast<int>(header.valueType), static_cast<int>(testCase.header.valueType));
}

INSTANTIATE_TEST_SUITE_P(
    FromBytesTests,
    TraceHeaderFromBytesTest,
    ::testing::Values(
    		TraceHeaderBytesTestCase{ {202, TraceMessageType::CONFIG, TraceValueType::UNKNOWN}, {0b0000'1100, 0b1010'0000} },
			TraceHeaderBytesTestCase{ {2, TraceMessageType::ARRAY_CONFIG, TraceValueType::UNKNOWN}, {0b0100'0000, 0b0010'0000} },
			TraceHeaderBytesTestCase{ {828, TraceMessageType::UPDATE, TraceValueType::UINT8}, {0b1011'0011, 0b1100'0100} },
			TraceHeaderBytesTestCase{ {959, TraceMessageType::UPDATE, TraceValueType::INT32}, {0b1011'1011, 0b1111'0111} },
			TraceHeaderBytesTestCase{ {33, TraceMessageType::UPDATE, TraceValueType::FLOAT}, {0b1000'0010, 0b0001'1011} },
			TraceHeaderBytesTestCase{ {386, TraceMessageType::ARRAY_UPDATE, TraceValueType::INT8}, {0b1101'1000, 0b0010'0011} },
			TraceHeaderBytesTestCase{ {149, TraceMessageType::ARRAY_UPDATE, TraceValueType::UINT16}, {0b1100'1001, 0b0101'0110} },
			TraceHeaderBytesTestCase{ {445, TraceMessageType::ARRAY_UPDATE, TraceValueType::DOUBLE}, {0b1101'1011, 0b1101'1100} }
    )
);
