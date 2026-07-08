#include <gtest/gtest.h>

#include "TraceDataHeader.h"

using namespace IntegralMotions::Tracing;

TEST(TraceDataHeaderTest, SizeCodeMatchesProtocolValues) {
    EXPECT_EQ(getTraceDataSizeCode(1), TraceDataSizeCode::ONE_BYTE);
    EXPECT_EQ(getTraceDataSizeCode(2), TraceDataSizeCode::TWO_BYTES);
    EXPECT_EQ(getTraceDataSizeCode(4), TraceDataSizeCode::FOUR_BYTES);
    EXPECT_EQ(getTraceDataSizeCode(8), TraceDataSizeCode::EIGHT_BYTES);
}

TEST(TraceDataHeaderTest, UnknownSizeFallsBackToOneByteCode) {
    EXPECT_EQ(getTraceDataSizeCode(0), TraceDataSizeCode::ONE_BYTE);
    EXPECT_EQ(getTraceDataSizeCode(3), TraceDataSizeCode::ONE_BYTE);
}

TEST(TraceDataHeaderTest, HeaderIsSingleByte) {
    EXPECT_EQ(sizeof(TraceDataHeader), 1U);
}
