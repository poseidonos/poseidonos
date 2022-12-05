#include <gmock/gmock.h>
#include <string>
#include <list>
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
