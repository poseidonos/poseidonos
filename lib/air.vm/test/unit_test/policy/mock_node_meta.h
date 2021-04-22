
#include "src/meta/NodeMeta.h"

class MockNodeMeta : public meta::NodeMeta
{
public:
    virtual ~MockNodeMeta()
    {
    }
    virtual inline void
    SetRun(bool new_run)
    {
        return;
    }
    virtual inline bool
    NodeEnable(uint32_t i) const
    {
        if (i == 2)
        {
            return node_status;
        }
        return true;
    }
    virtual inline void
    SetNodeEnable(uint32_t i, bool enable)
    {
        if (i == 2)
        { // for test
            node_status = true;
        }
        return;
    }
    virtual inline uint32_t
    NodeSampleRatio(uint32_t i) const
    {
        return 1;
    }
    virtual inline void
    SetNodeSampleRatio(uint32_t i, uint32_t sample_ratio)
    {
        return;
    }
    virtual inline void
    SetStreamingInterval(uint32_t new_streaming_interval)
    {
        return;
    }
    virtual inline void
    SetAirCore(uint32_t new_core)
    {
        return;
    }
    virtual inline bool
    Update() const
    {
        return true;
    }
    virtual inline void
    SetUpdate(bool new_update)
    {
        return;
    }
    virtual inline void
    SetNodeGroupId(uint32_t node_index, int32_t new_group_id)
    {
        group_id[node_index] = new_group_id;
    }
    virtual inline int32_t
    NodeGroupId(uint32_t node_index) const
    {
        return group_id[node_index];
    }

private:
    bool node_status{false};
    int32_t group_id[7]{
        -1,
    };
};
