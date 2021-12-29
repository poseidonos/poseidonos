#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/lib/signal_mask.h"

namespace pos
{
class MockSignalMask : public SignalMask
{
public:
    using SignalMask::SignalMask;
};

} // namespace pos
