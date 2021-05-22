
#include "src/collection/SwitchGear.h"

void
collection::SwitchGear::Run(void)
{
    for (auto& kv : node_manager->nda_map)
    {
        for (uint32_t nid = 0; nid < MAX_NID_SIZE; nid++)
        {
            if (air::ProcessorType::LATENCY == node_meta_getter->ProcessorType(nid))
            {
                uint32_t filter_size = node_meta_getter->FilterSize(nid);
                uint32_t index_size = node_meta_getter->IndexSize(nid);
                for (uint32_t hash_index = 0; hash_index < index_size; hash_index++)
                {
                    for (uint32_t filter_index = 0; filter_index < filter_size; filter_index++)
                    {
                        _CheckDeadline(kv.second->node[nid]->GetUserDataByHashIndex(hash_index, filter_index));
                    }
                }
            }
        }
    }
}

void
collection::SwitchGear::_CheckDeadline(lib::Data* data)
{
    lib::LatencyData* lat_data = static_cast<lib::LatencyData*>(data);

    if (lat_data->access)
    {
        if ((lib::TimeLogState::IDLE == lat_data->start_state) ||
            (lib::TimeLogState::RUN == lat_data->start_state))
        {
            lat_data->start_deadline--;
            if (-45 > lat_data->start_deadline)
            {
                lat_data->start_state = lib::TimeLogState::STOP;
            }
            else if (0 == lat_data->start_deadline)
            {
                lat_data->start_token = lat_data->start_size;
                lat_data->start_state = lib::TimeLogState::RUN;
            }
        }

        if ((lib::TimeLogState::IDLE == lat_data->end_state) ||
            (lib::TimeLogState::RUN == lat_data->end_state))
        {
            lat_data->end_deadline--;
            if (-45 > lat_data->end_deadline)
            {
                lat_data->end_state = lib::TimeLogState::STOP;
            }
            else if (0 == lat_data->end_deadline)
            {
                lat_data->end_token = lat_data->end_size;
                lat_data->end_state = lib::TimeLogState::RUN;
            }
        }
    }
}
