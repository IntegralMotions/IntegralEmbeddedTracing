#include <gtest/gtest.h>

#include "TraceDataHeader.h"

using namespace IntegralMotions::Tracing;

TEST(TraceDataHeaderTest, SizeCodeMatchesProtocolValues) {
    EXPECT_EQ(getTraceDataSizeCode(1), TraceDataSizeCode::OneByte);
    EXPECT_EQ(getTraceDataSizeCode(2), TraceDataSizeCode::TwoBytes);
    EXPECT_EQ(getTraceDataSizeCode(4), TraceDataSizeCode::FourBytes);
    EXPECT_EQ(getTraceDataSizeCode(8), TraceDataSizeCode::EightBytes);
}

TEST(TraceDataHeaderTest, UnknownSizeFallsBackToOneByteCode) {
    EXPECT_EQ(getTraceDataSizeCode(0), TraceDataSizeCode::OneByte);
    EXPECT_EQ(getTraceDataSizeCode(3), TraceDataSizeCode::OneByte);
}

TEST(TraceDataHeaderTest, HeaderIsSingleByte) {
    EXPECT_EQ(sizeof(TraceDataHeader), 1U);
}
