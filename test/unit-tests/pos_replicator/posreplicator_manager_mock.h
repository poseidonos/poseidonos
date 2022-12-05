#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/pos_replicator/posreplicator_manager.h"

namespace pos
{
class MockPosReplicatorManager : public PosReplicatorManager
{
public:
    using PosReplicatorManager::PosReplicatorManager;
};

} // namespace pos
