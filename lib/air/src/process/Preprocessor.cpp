
#include "src/process/Preprocessor.h"

#include <map>
#include <vector>

#include "src/lib/Protocol.h"

void
process::Preprocessor::Run(int option)
{
    uint32_t aid_size{global_meta_getter->AidSize()};
    uint32_t sid_size{lib::SID_SIZE};

    struct match_st
    {
        uint32_t run_state_count{0};
        std::vector<lib::LatencySeqData*> curr_v;
        std::vector<lib::LatencySeqData*> next_v;
    };

    std::map<uint32_t, struct match_st> match_map;
    match_map.clear();

    for (auto& kv : node_manager->thread_map)
    {
        for (uint32_t nid = 0; nid <= MAX_NID_SIZE - 1; nid++)
        {
            if (air::ProcessorType::LATENCY ==
                node_meta_getter->NodeProcessorType(nid))
            {
                for (uint32_t aid = 0; aid < aid_size; aid++)
                {
                    lib::LatencyData* lat_data = static_cast<lib::LatencyData*>(
                        kv.second.node[nid]->GetUserDataByAidIndex(aid));

                    if (lat_data->access)
                    {
                        for (uint32_t sid = 0; sid < sid_size - 1; sid++)
                        {
                            lib::LatencySeqData* sl_data_curr = &lat_data->seq_data[sid];
                            lib::LatencySeqData* sl_data_next = &lat_data->seq_data[sid + 1];

                            uint32_t key{0};
                            key = (nid << 8) + (aid << 16) + (sid);
                            struct match_st match_value;
                            match_value.run_state_count = 0;
                            match_value.curr_v.clear();
                            match_value.next_v.clear();

                            auto it_finder = match_map.find(key);
                            if (it_finder == match_map.end())
                            {
                                match_map.insert({key, match_value});
                            }

                            switch (sl_data_curr->start_state)
                            {
                                case (lib::TimeLogState::RUN):
                                    match_map[key].run_state_count += 1;
                                    break;
                                case (lib::TimeLogState::STOP):
                                case (lib::TimeLogState::FULL):
                                    if (!sl_data_curr->start_v.empty())
                                    {
                                        match_map[key].curr_v.push_back(&lat_data->seq_data[sid]);
                                    }
                                    break;
                                case (lib::TimeLogState::DONE):
                                case (lib::TimeLogState::IDLE):
                                default:
                                    break;
                            }

                            switch (sl_data_next->end_state)
                            {
                                case (lib::TimeLogState::RUN):
                                    match_map[key].run_state_count += 1;
                                    break;
                                case (lib::TimeLogState::STOP):
                                case (lib::TimeLogState::FULL):
                                    if (!sl_data_next->end_v.empty())
                                    {
                                        match_map[key].next_v.push_back(&lat_data->seq_data[sid + 1]);
                                    }
                                    break;
                                case (lib::TimeLogState::DONE):
                                case (lib::TimeLogState::IDLE):
                                default:
                                    break;
                            }
                        }
                    }
                }
            }
        }
    }

    for (auto& kv : match_map)
    {
        if (0 == kv.second.run_state_count)
        {
            if ((!kv.second.curr_v.empty()) && (!kv.second.next_v.empty()))
            {
                uint32_t aid = (kv.first >> 16);
                uint32_t nid = ((kv.first >> 8) & 0xFF);
                uint32_t sid = (kv.first & 0xFF);

                for (auto curr_v : kv.second.curr_v)
                {
                    for (auto next_v : kv.second.next_v)
                    {
                        _MatchKey(curr_v, next_v,
                            node_manager->GetAccLatSeqData(nid, aid, sid));
                    }
                }

                for (auto curr_v : kv.second.curr_v)
                {
                    for (auto next_v : kv.second.next_v)
                    {
                        curr_v->start_v.clear();
                        next_v->end_v.clear();
                    }
                }
            }
        }
    }
}

void
process::Preprocessor::_MatchKey(lib::LatencySeqData* curr_data,
    lib::LatencySeqData* next_data,
    lib::AccLatencySeqData* acc_data)
{
    curr_data->start_state = lib::TimeLogState::DONE;
    next_data->end_state = lib::TimeLogState::DONE;

    bool delete_flag{false};
    uint64_t timelag{0};

    for (auto it_start = curr_data->start_v.begin(); it_start != curr_data->start_v.end();)
    {
        for (auto it_end = next_data->end_v.begin(); it_end != next_data->end_v.end();)
        {
            if (it_start->key == it_end->key)
            {
                timelag =
                    NANOS * (it_end->timestamp.tv_sec - it_start->timestamp.tv_sec) +
                    (it_end->timestamp.tv_nsec - it_start->timestamp.tv_nsec);

                if (MAX_TIME > timelag)
                {
                    latency_writer.AddTimelag(acc_data, timelag);
                    curr_data->start_match_count++;
                    next_data->end_match_count++;
                }

                it_end = next_data->end_v.erase(it_end);
                delete_flag = true;
                break;
            }
            else
            {
                it_end++;
                delete_flag = false;
            }
        }

        if (delete_flag)
        {
            it_start = curr_data->start_v.erase(it_start);
        }
        else
        {
            it_start++;
        }
    }
}
