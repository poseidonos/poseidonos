#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/bio/flush_io.h"

namespace pos
{
class MockFlushIo : public FlushIo
{
public:
    using FlushIo::FlushIo;
    MOCK_METHOD(uint32_t, GetVolumeId, (), (override));
    MOCK_METHOD(uint32_t, GetOriginCore, (), (override));
};

} // namespace pos
