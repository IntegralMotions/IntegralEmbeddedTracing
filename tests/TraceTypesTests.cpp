#include <gtest/gtest.h>

#include "TraceTypes.h"

using namespace IntegralMotions::Tracing;

TEST(TraceTypesTest, ValueTypeSizesMatchProtocolTypes) {
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::UNKNOWN), 0U);
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::INT8), sizeof(int8_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::UINT8), sizeof(uint8_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::INT16), sizeof(int16_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::UINT16), sizeof(uint16_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::INT32), sizeof(int32_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::UINT32), sizeof(uint32_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::INT64), sizeof(int64_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::UINT64), sizeof(uint64_t));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::FLOAT), sizeof(float));
    EXPECT_EQ(getTraceValueTypeSize(TraceValueType::DOUBLE), sizeof(double));
}

TEST(TraceTypesTest, TemplateMapsKnownTypes) {
    EXPECT_EQ(getTraceValueType<int8_t>(), TraceValueType::INT8);
    EXPECT_EQ(getTraceValueType<uint8_t>(), TraceValueType::UINT8);
    EXPECT_EQ(getTraceValueType<int16_t>(), TraceValueType::INT16);
    EXPECT_EQ(getTraceValueType<uint16_t>(), TraceValueType::UINT16);
    EXPECT_EQ(getTraceValueType<int32_t>(), TraceValueType::INT32);
    EXPECT_EQ(getTraceValueType<uint32_t>(), TraceValueType::UINT32);
    EXPECT_EQ(getTraceValueType<int64_t>(), TraceValueType::INT64);
    EXPECT_EQ(getTraceValueType<uint64_t>(), TraceValueType::UINT64);
    EXPECT_EQ(getTraceValueType<float>(), TraceValueType::FLOAT);
    EXPECT_EQ(getTraceValueType<double>(), TraceValueType::DOUBLE);
}
