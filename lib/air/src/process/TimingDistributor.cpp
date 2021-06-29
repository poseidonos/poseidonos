
#include "src/process/TimingDistributor.h"

#include <map>

void
process::TimingDistributor::SetTiming(
    meta::NodeMetaGetter* node_meta_getter,
    node::NodeManager* node_manager)
{
    std::map<uint64_t, int32_t> timing_map;
    timing_map.clear();

    for (auto& kv : node_manager->nda_map)
    {
        for (uint32_t nid = 0; nid < MAX_NID_SIZE; nid++)
        {
            uint32_t index_size = node_meta_getter->IndexSize(nid);
            uint32_t filter_size = node_meta_getter->FilterSize(nid);

            if (node_meta_getter->ProcessorType(nid) ==
                air::ProcessorType::LATENCY)
            {
                for (uint32_t hash_index = 0; hash_index < index_size; hash_index++)
                {
                    for (uint32_t filter_index = 0; filter_index < filter_size - 1; filter_index++)
                    {
                        lib::LatencyData* from = static_cast<lib::LatencyData*>(
                            kv.second->node[nid]->GetUserDataByHashIndex(hash_index, filter_index));
                        lib::LatencyData* to = static_cast<lib::LatencyData*>(
                            kv.second->node[nid]->GetUserDataByHashIndex(hash_index, filter_index + 1));

                        uint64_t key{0};
                        int32_t value{0};
                        key = ((uint64_t)nid << 48) + ((uint64_t)hash_index << 32) +
                            ((uint64_t)filter_index);
                        value = dist(rand_engine);

                        auto it_finder = timing_map.find(key);
                        if (it_finder == timing_map.end())
                        {
                            timing_map.insert({key, value});
                        }

                        _ResetTiming(from, to, timing_map[key]);
                    }
                }
            }
        }
    }
}

void
process::TimingDistributor::_ResetTiming(lib::LatencyData* curr_data,
    lib::LatencyData* next_data, int32_t time_value)
{
    curr_data->start_deadline = time_value;
    if ((900 >= curr_data->start_size) && (MATCH_LOW_WM > curr_data->start_match_count))
    {
        curr_data->start_size += 100;
    }
    else if ((200 <= curr_data->start_size) && (MATCH_HIGH_WM < curr_data->start_match_count))
    {
        curr_data->start_size -= 100;
    }
    curr_data->start_match_count = 0;
    curr_data->start_v.clear();
    curr_data->start_state = lib::TimeLogState::IDLE;

    next_data->end_deadline = time_value;
    if ((900 >= next_data->end_size) && (MATCH_LOW_WM > next_data->end_match_count))
    {
        next_data->end_size += 100;
    }
    else if ((200 <= next_data->end_size) && (MATCH_HIGH_WM < next_data->end_match_count))
    {
        next_data->end_size -= 100;
    }
    next_data->end_match_count = 0;
    next_data->end_v.clear();
    next_data->end_state = lib::TimeLogState::IDLE;
}
