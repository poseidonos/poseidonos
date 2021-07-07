#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/gc/interface/i_gc_control.h"

namespace pos
{
class MockIGCControl : public IGCControl
{
public:
    using IGCControl::IGCControl;
    MOCK_METHOD(int, Start, (), (override));
    MOCK_METHOD(void, End, (), (override));
};

} // namespace pos
