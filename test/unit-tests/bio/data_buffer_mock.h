#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/bio/data_buffer.h"

namespace pos
{
class MockDataBuffer : public DataBuffer
{
public:
    using DataBuffer::DataBuffer;
};

} // namespace pos
