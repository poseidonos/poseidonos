
#ifndef AIR_NODE_META_H
#define AIR_NODE_META_H

#include "src/config/ConfigInterface.h"
#include "src/lib/Casting.h"
#include "src/lib/Type.h"

namespace meta
{
class NodeMeta
{
public:
    virtual ~NodeMeta(void)
    {
    }
    inline uint32_t
    NodeSize(void)
    {
        return sizeof(struct air::Node);
    }
    inline air::ProcessorType
    NodeProcessorType(uint32_t i) const
    {
        return node[i].processor_type;
    }
    inline bool
    Run(void) const
    {
        return run;
    }
    virtual inline bool
    Update(void) const
    {
        return update;
    }
    virtual inline bool
    NodeEnable(uint32_t i) const
    {
        return node[i].enable;
    }
    virtual inline uint32_t
    NodeSampleRatio(uint32_t i) const
    {
        return node[i].sample_ratio;
    }
    inline uint32_t
    NodeChildId(uint32_t i, uint32_t idx) const
    {
        return node[i].child_id[idx];
    }
    virtual inline int32_t
    NodeGroupId(uint32_t i) const
    {
        return node[i].group_id;
    }
    inline void*
    Meta(void)
    {
        return &node;
    }

    inline void
    SetNodeProcessorType(uint32_t i,
        air::ProcessorType processor_type)
    {
        node[i].processor_type = processor_type;
    }
    virtual inline void
    SetRun(bool new_run)
    {
        run = new_run;
        update = true;
    }
    virtual inline void
    SetUpdate(bool new_update)
    {
        update = new_update;
    }
    virtual inline void
    SetNodeEnable(uint32_t i, bool enable)
    {
        node[i].enable = enable;
        update = true;
    }
    virtual inline void
    SetNodeSampleRatio(uint32_t i, uint32_t sample_ratio)
    {
        node[i].sample_ratio = sample_ratio;
    }
    inline void
    SetNodeChildId(uint32_t i, uint32_t idx, uint32_t nid)
    {
        node[i].child_id[idx] = nid;
    }
    virtual inline void
    SetNodeGroupId(uint32_t i, int32_t group_id)
    {
        node[i].group_id = group_id;
    }

private:
    bool run{true};
    bool update{false};
    air::Node node[cfg::GetArrSize(config::ConfigType::NODE)];
};

class NodeMetaGetter
{
public:
    NodeMetaGetter(void)
    {
    }
    virtual ~NodeMetaGetter(void)
    {
    }
    explicit NodeMetaGetter(NodeMeta* new_node_meta)
    : node_meta(new_node_meta)
    {
    }
    inline uint32_t
    NodeSize(void)
    {
        return node_meta->NodeSize();
    }
    virtual inline air::ProcessorType
    NodeProcessorType(uint32_t i) const
    {
        return node_meta->NodeProcessorType(i);
    }
    inline bool
    Run(void) const
    {
        return node_meta->Run();
    }
    inline bool
    Update(void) const
    {
        return node_meta->Update();
    }
    virtual inline bool
    NodeEnable(uint32_t i) const
    {
        return node_meta->NodeEnable(i);
    }
    inline uint32_t
    NodeSampleRatio(uint32_t i) const
    {
        return node_meta->NodeSampleRatio(i);
    }
    inline uint32_t
    NodeChildId(uint32_t i, uint32_t idx) const
    {
        return node_meta->NodeChildId(i, idx);
    }
    virtual inline int32_t
    NodeGroupId(uint32_t i) const
    {
        return node_meta->NodeGroupId(i);
    }
    inline void*
    Meta(void)
    {
        return node_meta->Meta();
    }

private:
    NodeMeta* node_meta{nullptr};
};

} // namespace meta

#endif // AIR_NODE_META_H
