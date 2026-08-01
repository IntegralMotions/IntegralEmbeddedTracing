#include <gtest/gtest.h>

#include "TraceTypes.h"

using namespace IntegralMotions::Tracing;

TEST(TraceTypesTest, ValueTypeSizesMatchProtocolTypes) {
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::Unknown), 0U);
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::Int8), sizeof(int8_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::UInt8), sizeof(uint8_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::Int16), sizeof(int16_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::UInt16), sizeof(uint16_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::Int32), sizeof(int32_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::UInt32), sizeof(uint32_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::Int64), sizeof(int64_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::UInt64), sizeof(uint64_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::Float), sizeof(float));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::Double), sizeof(double));
}

TEST(TraceTypesTest, TemplateMapsKnownTypes) {
    EXPECT_EQ(getTraceValueType<int8_t>(), TraceValueType::Int8);
    EXPECT_EQ(getTraceValueType<uint8_t>(), TraceValueType::UInt8);
    EXPECT_EQ(getTraceValueType<int16_t>(), TraceValueType::Int16);
    EXPECT_EQ(getTraceValueType<uint16_t>(), TraceValueType::UInt16);
    EXPECT_EQ(getTraceValueType<int32_t>(), TraceValueType::Int32);
    EXPECT_EQ(getTraceValueType<uint32_t>(), TraceValueType::UInt32);
    EXPECT_EQ(getTraceValueType<int64_t>(), TraceValueType::Int64);
    EXPECT_EQ(getTraceValueType<uint64_t>(), TraceValueType::UInt64);
    EXPECT_EQ(getTraceValueType<float>(), TraceValueType::Float);
    EXPECT_EQ(getTraceValueType<double>(), TraceValueType::Double);
}
