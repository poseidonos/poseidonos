
#include "src/process/TimingDistributor.h"

#include <map>

void
process::TimingDistributor::SetTiming(
    meta::NodeMetaGetter* node_meta_getter,
    meta::GlobalMetaGetter* global_meta_getter,
    node::NodeManager* node_manager)
{
    uint32_t aid_size{global_meta_getter->AidSize()};
    uint32_t sid_size{lib::SID_SIZE};

    std::map<uint32_t, int32_t> timing_map;
    timing_map.clear();

    for (auto& kv : node_manager->thread_map)
    {
        for (uint32_t nid = 0; nid < MAX_NID_SIZE; nid++)
        {
            if (node_meta_getter->NodeProcessorType(nid) ==
                air::ProcessorType::LATENCY)
            {
                for (uint32_t aid = 0; aid <= aid_size - 1; aid++)
                {
                    lib::LatencyData* lat_data = static_cast<lib::LatencyData*>(
                        kv.second.node[nid]->GetUserDataByAidIndex(aid));

                    for (uint32_t sid = 0; sid < sid_size - 1; sid++)
                    {
                        uint32_t key{0};
                        int32_t value{0};
                        key = (nid << 8) + (aid << 16) + (sid);
                        value = dist(rand_engine);

                        lib::LatencySeqData* sl_data_curr = &lat_data->seq_data[sid];
                        lib::LatencySeqData* sl_data_next = &lat_data->seq_data[sid + 1];

                        auto it_finder = timing_map.find(key);
                        if (it_finder == timing_map.end())
                        {
                            timing_map.insert({key, value});
                        }

                        _ResetTiming(sl_data_curr, sl_data_next, timing_map[key]);
                    }
                }
            }
        }
    }
}

void
process::TimingDistributor::_ResetTiming(lib::LatencySeqData* curr_data,
    lib::LatencySeqData* next_data,
    int32_t time_value)
{
    curr_data->start_deadline = time_value;
    if ((900 >= curr_data->start_size) &&
        (MATCH_LOW_WM > curr_data->start_match_count))
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
    if ((900 >= next_data->end_size) &&
        (MATCH_LOW_WM > next_data->end_match_count))
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
