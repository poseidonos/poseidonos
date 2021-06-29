
#include "mock_global_meta.h"
#include "src/chain/ChainManager.cpp"
#include "src/chain/ChainManager.h"
#include "src/thread/Thread.cpp"

class ChainManagerTest : public ::testing::Test
{
public:
    chain::ChainManager* chain_manager{nullptr};
    MockGlobalMeta* mock_global_meta{nullptr};

protected:
    ChainManagerTest()
    {
        mock_global_meta = new MockGlobalMeta{};
        chain_manager = new chain::ChainManager{mock_global_meta};
    }
    ~ChainManagerTest() override
    {
        if (nullptr != mock_global_meta)
        {
            delete mock_global_meta;
            mock_global_meta = nullptr;
        }
        if (nullptr != chain_manager)
        {
            delete chain_manager;
            chain_manager = nullptr;
        }
    }
    void
    SetUp() override
    {
    }
    void
    TearDown() override
    {
    }
};
