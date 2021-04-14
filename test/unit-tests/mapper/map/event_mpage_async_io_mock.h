#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/map/event_mpage_async_io.h"

namespace pos
{
class MockEventMpageAsyncIo : public EventMpageAsyncIo
{
public:
    using EventMpageAsyncIo::EventMpageAsyncIo;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos
