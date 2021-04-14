#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/logger/deduplicator.h"

namespace pos_logger
{
class MockDeduplicator : public Deduplicator
{
public:
    using Deduplicator::Deduplicator;
};

} // namespace pos_logger
