#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log/gc_map_update_list.h"

namespace pos
{
class MockGcBlockMapUpdate : public GcBlockMapUpdate
{
public:
    using GcBlockMapUpdate::GcBlockMapUpdate;
};

class MockGcStripeMapUpdateList : public GcStripeMapUpdateList
{
public:
    using GcStripeMapUpdateList::GcStripeMapUpdateList;
};

} // namespace pos
