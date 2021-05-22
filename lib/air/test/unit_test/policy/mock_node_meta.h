
#include "src/meta/NodeMeta.h"

class MockNodeMeta : public meta::NodeMeta
{
public:
    virtual ~MockNodeMeta()
    {
    }
    virtual inline void
    SetRun(uint32_t nid, bool new_run)
    {
        run[nid] = new_run;
    }
    virtual inline bool
    Run(uint32_t nid)
    {
        return run[nid];
    }
    virtual inline uint32_t
    SampleRatio(uint32_t i) const
    {
        return 1;
    }
    virtual inline void
    SetSampleRatio(uint32_t i, uint32_t sample_ratio)
    {
        return;
    }
    virtual inline void
    SetGroupId(uint32_t node_index, uint32_t new_group_id)
    {
        group_id[node_index] = new_group_id;
    }
    virtual inline uint32_t
    GroupId(uint32_t node_index) const
    {
        return group_id[node_index];
    }

private:
    bool run[7]{
        false,
    };
    uint32_t group_id[7]{0, 1, 0, 0, 1, 1, 1};
};
