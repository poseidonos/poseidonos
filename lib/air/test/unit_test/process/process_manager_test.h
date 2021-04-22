
#include "fake_global_meta_getter.h"
#include "fake_node_manager.h"
#include "fake_node_meta_getter.h"
#include "src/process/ProcessManager.cpp"
#include "src/process/ProcessManager.h"
#include "src/process/TimingDistributor.cpp"
#include "src/process/TimingDistributor.h"

class ProcessManagerTest : public ::testing::Test
{
public:
    process::ProcessManager* process_manager{nullptr};

    FakeGlobalMetaGetter* fake_global_meta_getter{nullptr};
    FakeNodeMetaGetter* fake_node_meta_getter{nullptr};
    FakeNodeManager* fake_node_manager{nullptr};

protected:
    ProcessManagerTest()
    {
        fake_global_meta_getter = new FakeGlobalMetaGetter;
        fake_node_meta_getter = new FakeNodeMetaGetter;

        fake_node_manager = new FakeNodeManager(
            fake_global_meta_getter, fake_node_meta_getter);

        process_manager = new process::ProcessManager(
            fake_global_meta_getter, fake_node_meta_getter,
            fake_node_manager);
    }
    virtual ~ProcessManagerTest()
    {
        if (nullptr != fake_global_meta_getter)
        {
            delete fake_global_meta_getter;
            fake_global_meta_getter = nullptr;
        }
        if (nullptr != fake_node_meta_getter)
        {
            delete fake_node_meta_getter;
            fake_node_meta_getter = nullptr;
        }
        if (nullptr != process_manager)
        {
            delete process_manager;
            process_manager = nullptr;
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
