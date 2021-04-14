
#ifndef AIR_NODE_MANAGER_H
#define AIR_NODE_MANAGER_H

#include <pthread.h>

#include <map>
#include <string>

#include "src/config/ConfigInterface.h"
#include "src/lib/Casting.h"
#include "src/meta/GlobalMeta.h"
#include "src/meta/NodeMeta.h"
#include "src/profile_data/node/NodeThread.h"

#define NAMELEN 16

namespace node
{
struct ThreadArray
{
    Thread* node[cfg::GetArrSize(config::ConfigType::NODE)];
    std::string tname{NULL};
};

class NodeManager
{
public:
    NodeManager(void)
    {
    }
    NodeManager(meta::GlobalMetaGetter* new_global_meta_getter,
        meta::NodeMetaGetter* new_node_meta_getter)
    : global_meta_getter(new_global_meta_getter),
      node_meta_getter(new_node_meta_getter)
    {
        thread_map.clear();
    }
    virtual ~NodeManager(void)
    {
        std::map<uint32_t, node::ThreadArray>::iterator it;
        it = thread_map.begin();
        while (it != thread_map.end())
        {
            DeleteThread(&(it->second));
            thread_map.erase(it++);
        }
        thread_map.clear();
        for (uint32_t i = 0; i < MAX_NID_SIZE; i++)
        {
            delete[] acc_lat_data[i];
        }
        delete[] acc_lat_data;
    }

    virtual ThreadArray* GetThread(uint32_t tid);
    virtual void SetThreadName(uint32_t tid);
    virtual int CreateThread(uint32_t tid);
    virtual void DeleteThread(ThreadArray* thread_array);
    virtual bool CanDelete(ThreadArray* thread_array);

    std::map<uint32_t, ThreadArray> thread_map;

    void Init(void);
    virtual lib::AccLatencySeqData*
    GetAccLatSeqData(uint32_t nid, uint32_t aid,
        uint32_t sid)
    {
        return &(acc_lat_data[nid][aid].seq_data[sid]);
    }
    virtual lib::AccLatencyData*
    GetAccLatData(uint32_t nid, uint32_t aid)
    {
        return &(acc_lat_data[nid][aid]);
    }

private:
    meta::GlobalMetaGetter* global_meta_getter{nullptr};
    meta::NodeMetaGetter* node_meta_getter{nullptr};
    lib::AccLatencyData** acc_lat_data{
        nullptr,
    };
    const uint32_t MAX_NID_SIZE{cfg::GetArrSize(config::ConfigType::NODE)};
};

} // namespace node

#endif // AIR_NODE_MANAGER_H
