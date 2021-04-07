
#ifndef AIR_COLLECTION_MANAGER_H
#define AIR_COLLECTION_MANAGER_H

#include <queue>

#include "src/collection/Collector.h"
#include "src/config/ConfigInterface.h"
#include "src/lib/Design.h"
#include "src/lib/Msg.h"
#include "src/meta/GlobalMeta.h"
#include "src/meta/NodeMeta.h"
#include "src/profile_data/node/NodeManager.h"

namespace collection
{
class Subject : public lib_design::Subject
{
public:
    Subject(void)
    {
    }
    virtual ~Subject(void)
    {
    }
    virtual int Notify(uint32_t index, uint32_t type1, uint32_t type2,
        uint32_t value1, uint32_t value2, int pid, int cmd_type,
        int cmd_order);
};

class CollectionManager
{
public:
    CollectionManager(meta::GlobalMetaGetter* new_global_meta_getter,
        meta::NodeMetaGetter* new_node_meta_getter,
        node::NodeManager* new_node_manager,
        collection::Subject* new_subject)
    : global_meta_getter(new_global_meta_getter),
      node_meta_getter(new_node_meta_getter),
      node_manager(new_node_manager),
      subject(new_subject)
    {
    }
    virtual ~CollectionManager(void);
    void Init(void);

    node::ThreadArray*
    GetThread(uint32_t tid)
    {
        return node_manager->GetThread(tid);
    }

    int
    CreateThread(uint32_t tid)
    {
        // 0 -> already create
        // 1 -> create complete
        return node_manager->CreateThread(tid);
    }

    virtual inline bool
    IsLog(uint32_t nid)
    {
        return (air_enable & node_enable[nid]);
    }

    virtual inline void
    LogData(uint32_t nid, uint32_t aid,
        node::ThreadArray* thread_array, uint32_t value1,
        uint64_t value2)
    {
        if (nullptr != thread_array)
        {
            node::Thread* thr = thread_array->node[nid];
            if (nullptr == thr)
            {
                return;
            }
            lib::Data* user_data = thr->GetUserDataByAidValue(aid);
            if (nullptr == user_data)
            {
                return;
            }
            // thread-aware data
            thr->SetIsLogging(true);
            collector[nid]->LogData(user_data, value1, value2);
        }
    }

    void HandleMsg(void);
    int UpdateCollection(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2);
    void
    EnqueueMsg(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2, int pid, int cmd_type, int cmd_order)
    {
        int dummy = 0;
        msg.push({type1, type2, value1, value2, pid, cmd_type, cmd_order});
        dummy++;
    }

private:
    int _EnableNode(uint32_t node_index, uint32_t value2);
    int _EnableRangeNode(uint32_t start_idx, uint32_t end_idx, uint32_t value2);
    int _EnableGroupNode(uint32_t gid, uint32_t value2);
    int _UpdateEnable(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2);
    void _InitNode(uint32_t node_index);
    int _UpdateInit(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2);
    int _UpdateNodeSamplingRate(uint32_t node_index, uint32_t new_ratio);
    int _UpdateSamplingRate(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2);

    Collector* collector[cfg::GetArrSize(config::ConfigType::NODE)]{
        nullptr,
    };

    meta::GlobalMetaGetter* global_meta_getter{nullptr};
    bool air_enable{false};

    meta::NodeMetaGetter* node_meta_getter{nullptr};
    bool node_enable[cfg::GetArrSize(config::ConfigType::NODE)]{
        false,
    };

    node::NodeManager* node_manager{nullptr};

    uint32_t max_aid_size{0};
    const uint32_t MAX_NID_SIZE{cfg::GetArrSize(config::ConfigType::NODE)};
    const uint32_t MAX_SEQ_IDX{10};

    std::queue<lib::MsgEntry> msg;

    Subject* subject{nullptr};
};

} // namespace collection

#endif // AIR_COLLECTION_MANAGER_H
