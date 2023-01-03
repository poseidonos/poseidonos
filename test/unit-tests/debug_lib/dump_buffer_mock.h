#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/debug_lib/dump_buffer.h"

namespace pos
{
class MockDumpBufferDeleter : public DumpBufferDeleter
{
public:
    using DumpBufferDeleter::DumpBufferDeleter;
};

class MockDumpBuffer : public DumpBuffer
{
public:
    using DumpBuffer::DumpBuffer;
};

} // namespace pos
