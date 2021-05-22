
#include "src/api/Air.h"
#include "src/collection/CollectionManager.cpp"
#include "src/collection/CollectionManager.h"
#include "src/collection/Collector.cpp"
#include "src/collection/Collector.h"
#include "src/collection/writer/LatencyWriter.cpp"
#include "src/collection/writer/LatencyWriter.h"
#include "src/collection/writer/PerformanceWriter.cpp"
#include "src/collection/writer/PerformanceWriter.h"
#include "src/collection/writer/QueueWriter.cpp"
#include "src/collection/writer/QueueWriter.h"
#include "src/collection/writer/Writer.cpp"
#include "src/collection/writer/Writer.h"
#include "src/config/ConfigInterface.h"

class FakeCollectionManager : public collection::CollectionManager
{
public:
    FakeCollectionManager(
        meta::GlobalMetaGetter* new_global_meta_getter,
        meta::NodeMetaGetter* new_node_meta_getter,
        node::NodeManager* new_node_manager,
        collection::Subject* new_subject)
    :

      collection::CollectionManager(
          new_global_meta_getter,
          new_node_meta_getter,
          new_node_manager,
          new_subject)
    {
    }

    bool
    IsLog(uint32_t new_nid) override
    {
        if (cfg::GetSentenceCount(config::ParagraphType::NODE) >= new_nid)
        {
            nid = new_nid;
            return true;
        }
        else
        {
            return false;
        }
    }
    void
    LogData(uint32_t new_nid, uint32_t new_filter_index,
        node::NodeDataArray* node_data_array, uint64_t new_node_index,
        uint64_t new_value) override
    {
        nid = new_nid;
        filter_index = new_filter_index;
        node_index = new_node_index;
        value = new_value;
        return;
    }
    uint32_t nid{0};
    uint64_t filter_index{0};
    uint64_t node_index{0};
    uint64_t value{0};
};
