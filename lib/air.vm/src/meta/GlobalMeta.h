
#ifndef AIR_GLOBAL_META_H
#define AIR_GLOBAL_META_H

#include <cstdint>

#include "src/config/ConfigInterface.h"

namespace meta
{
class GlobalMeta
{
public:
    virtual ~GlobalMeta(void)
    {
    }
    inline bool
    AirBuild(void) const
    {
        return air_build;
    }
    inline bool
    AirPlay(void) const
    {
        return air_play;
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
    SetAirBuild(bool new_air_build)
    {
        air_build = new_air_build;
    }
    virtual inline void
    SetAirPlay(bool new_air_play)
    {
        air_play = new_air_play;
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

private:
    bool air_build{true};
    bool air_play{true};
    uint32_t streaming_interval{1};
    bool streaming_update{false};
    uint32_t streaming_value{0};
    uint32_t cpu_num{0};
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
    AirBuild(void) const
    {
        return global_meta->AirBuild();
    }
    virtual inline bool
    AirPlay(void) const
    {
        return global_meta->AirPlay();
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

private:
    GlobalMeta* global_meta{nullptr};
};

} // namespace meta

#endif // AIR_GLOBAL_META_H
