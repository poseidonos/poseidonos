
#ifndef AIR_NODE_MANAGER_H
#define AIR_NODE_MANAGER_H

#include <pthread.h>

#include <map>
#include <string>

#include "src/config/ConfigInterface.h"
#include "src/data_structure/NodeData.h"
#include "src/lib/Casting.h"
#include "src/meta/GlobalMeta.h"
#include "src/meta/NodeMeta.h"

#define NAMELEN 100

namespace node
{
struct NodeDataArray
{
    NodeData* node[cfg::GetSentenceCount(config::ParagraphType::NODE)];
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
        nda_map.clear();
    }
    virtual ~NodeManager(void)
    {
        auto iter = nda_map.begin();
        while (iter != nda_map.end())
        {
            DeleteNodeDataArray(iter->second);
            nda_map.erase(iter++);
        }
        nda_map.clear();
        for (uint32_t nid = 0; nid < MAX_NID_SIZE; nid++)
        {
            if (air::ProcessorType::LATENCY == node_meta_getter->ProcessorType(nid))
            {
                const uint32_t index_size = node_meta_getter->IndexSize(nid);
                for (uint32_t hash_index = 0; hash_index < index_size; hash_index++)
                {
                    delete[] acc_lat_data[nid][hash_index];
                }
            }
        }
        for (uint32_t nid = 0; nid < MAX_NID_SIZE; nid++)
        {
            if (air::ProcessorType::LATENCY == node_meta_getter->ProcessorType(nid))
            {
                delete[] acc_lat_data[nid];
            }
        }
        delete[] acc_lat_data;
    }

    virtual NodeDataArray* GetNodeDataArray(uint32_t tid);
    virtual void SetNodeDataArrayName(uint32_t tid);
    virtual void CreateNodeDataArray(uint32_t tid);
    virtual void DeleteNodeDataArray(NodeDataArray* node_data_array);
    virtual bool CanDeleteNodeDataArray(NodeDataArray* node_data_array);

    std::map<uint32_t, NodeDataArray*> nda_map;

    void Init(void);

    virtual lib::AccLatencyData*
    GetAccLatData(uint32_t nid, uint32_t hash_index, uint32_t filter_index)
    {
        if (node_meta_getter->IndexSize(nid) <= hash_index ||
            node_meta_getter->FilterSize(nid) <= filter_index ||
            air::ProcessorType::LATENCY != node_meta_getter->ProcessorType(nid))
        {
            return nullptr;
        }
        return &(acc_lat_data[nid][hash_index][filter_index]);
    }

private:
    meta::GlobalMetaGetter* global_meta_getter{nullptr};
    meta::NodeMetaGetter* node_meta_getter{nullptr};
    lib::AccLatencyData*** acc_lat_data{
        nullptr,
    };
    const uint32_t MAX_NID_SIZE{cfg::GetSentenceCount(config::ParagraphType::NODE)};
};

} // namespace node

#endif // AIR_NODE_MANAGER_H
