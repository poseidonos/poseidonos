
#ifndef AIR_GLOBAL_META_H
#define AIR_GLOBAL_META_H

#include <cstdint>

namespace meta
{
class GlobalMeta
{
public:
    virtual ~GlobalMeta(void)
    {
    }
    inline bool
    Enable(void) const
    {
        return enable;
    }
    virtual inline uint32_t
    StreamingInterval(void) const
    {
        return streaming_interval;
    }
    virtual inline uint32_t
    NextStreamingInterval(void) const
    {
        return streaming_value;
    }
    virtual inline bool
    StreamingUpdate(void) const
    {
        return streaming_update;
    }
    virtual inline void
    SetEnable(bool new_enable)
    {
        enable = new_enable;
    }
    virtual inline void
    SetStreamingInterval(uint32_t new_streaming_interval)
    {
        streaming_value = new_streaming_interval;
        streaming_update = true;
    }
    virtual inline void
    UpdateStreamingInterval(void)
    {
        streaming_interval = streaming_value;
        streaming_update = false;
    }
    inline void
    SetCpuNum(uint32_t new_cpu_num)
    {
        cpu_num = new_cpu_num;
    }
    inline uint32_t
    CpuNum(void) const
    {
        return cpu_num;
    }
    inline void
    SetAidSize(uint32_t new_aid_size)
    {
        aid_size = new_aid_size;
    }
    inline uint32_t
    AidSize(void) const
    {
        return aid_size;
    }

private:
    bool enable{true};
    uint32_t streaming_interval{1};
    bool streaming_update{false};
    uint32_t streaming_value{0};
    uint32_t cpu_num{0};
    uint32_t aid_size{0};
};

class GlobalMetaGetter
{
public:
    GlobalMetaGetter(void)
    {
    }
    virtual ~GlobalMetaGetter(void)
    {
    }
    explicit GlobalMetaGetter(GlobalMeta* new_global_meta)
    : global_meta(new_global_meta)
    {
    }
    virtual inline bool
    Enable(void) const
    {
        return global_meta->Enable();
    }
    virtual inline uint32_t
    StreamingInterval(void) const
    {
        return global_meta->StreamingInterval();
    }
    virtual inline uint32_t
    NextStreamingInterval(void) const
    {
        return global_meta->NextStreamingInterval();
    }
    inline bool
    StreamingUpdate(void) const
    {
        return global_meta->StreamingUpdate();
    }
    inline uint32_t
    CpuNum(void) const
    {
        return global_meta->CpuNum();
    }
    virtual inline uint32_t
    AidSize(void) const
    {
        return global_meta->AidSize();
    }

private:
    GlobalMeta* global_meta{nullptr};
};

} // namespace meta

#endif // AIR_GLOBAL_META_H
