
#ifndef AIR_NODE_THREAD_H
#define AIR_NODE_THREAD_H

#include "src/lib/Data.h"
#include "src/lib/Hash.h"
#include "src/lib/Type.h"

namespace node
{
class Thread
{
public:
    Thread(void)
    {
    }
    Thread(air::ProcessorType new_type, uint32_t new_aid_size);
    virtual ~Thread(void);
    virtual inline lib::Data*
    GetAirData(uint32_t aid_idx)
    {
        if (aid_idx >= max_aid_size)
        {
            return nullptr;
        }
        return air_data[aid_idx];
    }
    virtual inline lib::Data*
    GetUserDataByAidValue(uint32_t aid_value)
    {
        uint32_t aid_idx = hash_map->GetHashIndex(aid_value);
        if (aid_idx == max_aid_size)
        {
            aid_idx = hash_map->InsertHashNode(aid_value);
            if (aid_idx == max_aid_size)
            { // hash map is full
                return nullptr;
            }
        }
        return user_data[aid_idx];
    }
    virtual inline lib::Data*
    GetUserDataByAidIndex(uint32_t aid_idx)
    {
        if (aid_idx >= max_aid_size)
        {
            return nullptr;
        }
        return user_data[aid_idx];
    }
    virtual inline uint32_t
    GetUserAidValue(uint32_t aid_idx)
    {
        return hash_map->GetHashKey(aid_idx);
    }
    virtual inline lib::AccData*
    GetAccData(uint32_t aid_idx)
    {
        if (aid_idx >= max_aid_size)
        {
            return nullptr;
        }
        return acc_data[aid_idx];
    }
    inline void
    SetIsLogging(bool new_is_logging)
    {
        is_logging = new_is_logging;
    }
    inline bool
    IsLogging(void) const
    {
        return is_logging;
    }
    virtual inline void
    SwapBuffer(uint32_t aid_idx)
    {
        if (aid_idx >= max_aid_size)
        {
            return;
        }
        lib::Data* tmp_addr = user_data[aid_idx];
        user_data[aid_idx] = air_data[aid_idx];
        air_data[aid_idx] = tmp_addr;
    }

private:
    air::ProcessorType ptype{air::ProcessorType::PROCESSORTYPE_NULL};
    lib::Data** user_data{
        nullptr,
    };
    lib::Data** air_data{
        nullptr,
    };
    lib::AccData** acc_data{
        nullptr,
    };
    uint32_t max_aid_size{0};
    bool is_logging{false};
    hash::HashMap<uint32_t>* hash_map{nullptr};
};

} // namespace node

#endif // AIR_NODE_THREAD_H
