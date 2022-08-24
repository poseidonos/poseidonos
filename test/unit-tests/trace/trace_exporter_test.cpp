#include <gtest/gtest.h>

#include "src/include/pos_event_id.h"
#include "src/trace/trace_exporter.h"
using namespace pos;

TEST(TraceExporter, Init_)
{
    // Given
    TraceExporter te;
    
    // When
    te.Init("TestService", "TestVersion", "test.endpoint");

    // Then
    ASSERT_TRUE(te.IsEnabled());
}