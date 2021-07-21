#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/ft/buffer_entry.h"

namespace pos
{
class MockBufferEntry : public BufferEntry
{
public:
    using BufferEntry::BufferEntry;
    MOCK_METHOD(void, ReturnBuffer, (), (override));
};

} // namespace pos
