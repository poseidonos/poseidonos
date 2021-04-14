
#include "src/collection/SwitchGear.h"

void
collection::SwitchGear::Run(void)
{
    uint32_t aid_size{global_meta_getter->AidSize()};

    for (auto& kv : node_manager->thread_map)
    {
        for (uint32_t nid = 0; nid < MAX_NID_SIZE; nid++)
        {
            if (air::ProcessorType::LATENCY ==
                node_meta_getter->NodeProcessorType(nid))
            {
                for (uint32_t aid = 0; aid < aid_size; aid++)
                {
                    _CheckDeadline(kv.second.node[nid]->GetUserDataByAidIndex(aid));
                }
            }
        }
    }
}

void
collection::SwitchGear::_CheckDeadline(lib::Data* data)
{
    uint32_t sid_size{lib::SID_SIZE};

    lib::LatencyData* lat_data = static_cast<lib::LatencyData*>(data);

    if (lat_data->access)
    {
        for (uint32_t i = 0; i < sid_size; i++)
        {
            lib::LatencySeqData* sl_data =
                static_cast<lib::LatencySeqData*>(&lat_data->seq_data[i]);

            if ((lib::TimeLogState::IDLE == sl_data->start_state) ||
                (lib::TimeLogState::RUN == sl_data->start_state))
            {
                sl_data->start_deadline--;
                if (-45 > sl_data->start_deadline)
                {
                    sl_data->start_state = lib::TimeLogState::STOP;
                }
                else if (0 == sl_data->start_deadline)
                {
                    sl_data->start_token = sl_data->start_size;
                    sl_data->start_state = lib::TimeLogState::RUN;
                }
            }

            if ((lib::TimeLogState::IDLE == sl_data->end_state) ||
                (lib::TimeLogState::RUN == sl_data->end_state))
            {
                sl_data->end_deadline--;
                if (-45 > sl_data->end_deadline)
                {
                    sl_data->end_state = lib::TimeLogState::STOP;
                }
                else if (0 == sl_data->end_deadline)
                {
                    sl_data->end_token = sl_data->end_size;
                    sl_data->end_state = lib::TimeLogState::RUN;
                }
            }
        }
    }
}
