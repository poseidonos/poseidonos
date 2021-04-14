
#include "src/profile_data/node/NodeThread.h"
#include "src/profile_data/node/NodeThread.cpp"
#include "src/lib/Hash.cpp"

class NodeThreadTest : public ::testing::Test
{
public:
    node::Thread* thread {nullptr};

protected:
    NodeThreadTest() {}
    ~NodeThreadTest() override {
        if (nullptr != thread) {
            delete thread;
            thread = nullptr;
        }
    }
    void SetUp() override {}
    void TearDown() override {}
};
