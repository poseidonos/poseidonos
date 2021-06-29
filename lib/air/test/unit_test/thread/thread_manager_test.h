
#include "mock_chain_manager.h"
#include "src/thread/ThreadManager.cpp"
#include "src/thread/ThreadManager.h"

class ThreadManagerTest : public ::testing::Test
{
public:
    MockChainManager* mock_chain_manager{nullptr};
    thread::ThreadManager* thread_manager{nullptr};

protected:
    ThreadManagerTest()
    {
        mock_chain_manager = new MockChainManager{};
        thread_manager = new thread::ThreadManager{mock_chain_manager};
    }
    ~ThreadManagerTest()
    {
        if (nullptr != mock_chain_manager)
        {
            delete mock_chain_manager;
            mock_chain_manager = nullptr;
        }
        if (nullptr != thread_manager)
        {
            delete thread_manager;
            thread_manager = nullptr;
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
