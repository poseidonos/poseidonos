
#include "src/config/ConfigInterface.h"
#include "src/lib/Type.h"

class TypeTest : public ::testing::Test
{
public:
    air::NodeMetaData node_meta{};

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
        node_meta.nid = cfg::GetSentenceIndex(config::ParagraphType::NODE, "Q_COMPLETION");
        node_meta.processor_type = air::ProcessorType::QUEUE;
        node_meta.run = true;
        node_meta.group_id = 1;
        node_meta.index_size = 10;
        ;
        node_meta.filter_size = 10;
        node_meta.sample_ratio = 1000;
    }
    void
    TearDown() override
    {
    }
};
