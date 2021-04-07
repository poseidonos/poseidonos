
#include "src/api/AirTemplate.h"
#include "src/config/ConfigInterface.h"
#include "src/collection/Writer.h"
#include "src/collection/Writer.cpp"
#include "src/collection/Collector.h"
#include "src/collection/Collector.cpp"
#include "src/collection/CollectionManager.h"
#include "src/collection/CollectionManager.cpp"

class FakeCollectionManager : public collection::CollectionManager
{
public:
    FakeCollectionManager(
        meta::GlobalMetaGetter* new_global_meta_getter,
        meta::NodeMetaGetter* new_node_meta_getter,
        node::NodeManager* new_node_manager,
        collection::Subject* new_subject):

        collection::CollectionManager(
            new_global_meta_getter,
            new_node_meta_getter,
            new_node_manager,
            new_subject) {}

    bool IsLog(uint32_t new_nid) override {
        if (cfg::GetArrSize(config::ConfigType::NODE) >= new_nid) {
            nid = new_nid;
            return true;
        }
        else {
            return false;
        }
    }
    void LogData(uint32_t new_nid, uint32_t new_aid,
                 node::ThreadArray* thread_array, uint32_t new_value1,
                 uint64_t new_value2) override {
        nid = new_nid;
        aid = new_aid;
        value1 = new_value1;
        value2 = new_value2;
        return;
    }
    uint32_t nid {0};
    uint32_t aid {0};
    uint32_t value1 {0};
    uint64_t value2 {0};
};
