
#include "src/config/ConfigInterface.h"
#include "src/lib/Type.h"

class TypeTest : public ::testing::Test
{
public:
    air::Node node{};

protected:
    TypeTest()
    {
    }
    ~TypeTest() override
    {
    }
    void
    SetUp() override
    {
        node.nid = cfg::GetIndex(config::ConfigType::NODE, "Q_COMPLETION");
        node.processor_type = air::ProcessorType::QUEUE;
        node.enable = true;
    }
    void
    TearDown() override
    {
    }
};
