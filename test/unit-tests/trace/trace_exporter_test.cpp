#include <gtest/gtest.h>

#include "src/include/pos_event_id.h"
#include "test/unit-tests/trace/trace_exporter_mock.h"

using namespace pos;

TEST(TraceExporter, Init_)
{
    // Given
    MockTraceExporter te;

    // When
    te.Init("TestService", "TestVersion", "test.endpoint");

    // Then
    ASSERT_TRUE(te.IsEnabled());
}