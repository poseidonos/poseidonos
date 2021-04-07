
#ifndef AIR_RULER_H
#define AIR_RULER_H

#include "src/config/ConfigInterface.h"
#include "src/lib/Protocol.h"
#include "src/meta/GlobalMeta.h"
#include "src/meta/NodeMeta.h"

namespace policy
{
class Ruler
{
public:
    Ruler(meta::NodeMeta* new_node_meta, meta::GlobalMeta* new_global_meta)
    : node_meta(new_node_meta),
      global_meta(new_global_meta)
    {
    }
    int CheckRule(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2);
    bool SetRule(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2);
    bool
    Update(void)
    {
        return node_meta->Update();
    }
    void
    SetUpdate(bool update)
    {
        node_meta->SetUpdate(update);
    }
    void
    SetStreamingInterval(int streaming_interval)
    {
        global_meta->SetStreamingInterval(streaming_interval);
    }
    void
    SetCpuNum(uint32_t cpu_num)
    {
        global_meta->SetCpuNum(cpu_num);
    }
    void
    SetAidSize(uint32_t aid_num)
    {
        global_meta->SetAidSize(aid_num);
    }

private:
    int _CheckEnableNodeRule(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2);
    int _CheckInitNodeRule(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2);
    int _CheckSetSamplingRatioRule(uint32_t type1, uint32_t type2,
        uint32_t value1, uint32_t value2);
    int _CheckStreamInterval(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2);
    bool _SetEnableNodeRule(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2);
    // bool SetInitNodeRule(uint32_t type1, uint32_t type2,
    //                     uint32_t value1, uint32_t value2);
    bool _SetSamplingRatioRule(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2);

    meta::NodeMeta* node_meta{nullptr};
    meta::GlobalMeta* global_meta{nullptr};
    const uint32_t MAX_NID_SIZE{cfg::GetArrSize(config::ConfigType::NODE)};
};

} // namespace policy

#endif // AIR_RULER_H
