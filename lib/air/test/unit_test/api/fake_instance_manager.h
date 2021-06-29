
#include "src/api/Air.h"

air::InstanceManager::InstanceManager()
{
}
air::InstanceManager::~InstanceManager()
{
}
int
air::InstanceManager::Initialize(uint32_t cpu_num)
{
    return 0;
}
int
air::InstanceManager::Finalize()
{
    return 0;
}
int
air::InstanceManager::Activate()
{
    return 0;
}
int
air::InstanceManager::Deactivate()
{
    return 0;
}

class FakeInstanceManager : public air::InstanceManager
{
public:
    FakeInstanceManager()
    : air::InstanceManager()
    {
    }

    int
    Initialize(uint32_t cpu_num) override
    {
        return 0;
    }
    int
    Finalize() override
    {
        return 0;
    }
    int
    Activate() override
    {
        return 0;
    }
    int
    Deactivate() override
    {
        return 0;
    }
    node::NodeManager*
    GetNodeManager() override
    {
        return nullptr;
    }
    collection::CollectionManager*
    GetCollectionManager() override
    {
        return nullptr;
    }
};
