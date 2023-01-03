#include "src/debug_lib/dump_buffer.h"

#include <gtest/gtest.h>


namespace pos
{
class DummyBufferTest
{
private:
    int i;
    int j;
};

TEST(DumpBufferDeleter, operator)
{
    DumpBufferDeleter dumpBufferDeleter;
    uint8_t* buf = new uint8_t[10];
    dumpBufferDeleter(buf);
}

TEST(DumpBuffer, Constructor)
{
    // Given : New DummyBufferTest is created and dump module is created
    DummyBufferTest* dummyBuffer = new DummyBufferTest;
    DebugInfoQueue<DumpBuffer> dumpModule("test", 10, true);
    // When : DumpBuffer is created with dump Module
    DumpBuffer dumpBuffer(dummyBuffer, 10, &dumpModule);

    // Then : New dumpBuffer is created without param
    DumpBuffer dumpBuffer3;

    // Given : New dumpModule is created
    DebugInfoQueue<DumpBuffer> dumpModule2("test2", 10, false);
    // When : DumpBuffer is created with dump Module
    DumpBuffer dumpBuffer2(dummyBuffer, 10, &dumpModule2);
    delete dummyBuffer;
}

} // namespace pos
