
#include "src/chain/ChainManager.cpp"
#include "src/chain/ChainManager.h"

class MockChainManager : public chain::ChainManager
{
public:
    virtual ~MockChainManager()
    {
    }
    virtual void
    RunThread(uint32_t run_skip_count)
    {
        return;
    }

private:
};
