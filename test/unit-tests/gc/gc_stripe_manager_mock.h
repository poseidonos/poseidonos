#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/gc/gc_stripe_manager.h"

namespace pos
{
class MockGcStripeManager : public GcStripeManager
{
public:
    using GcStripeManager::GcStripeManager;
};

} // namespace pos
