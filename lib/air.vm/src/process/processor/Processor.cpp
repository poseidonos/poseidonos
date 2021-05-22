
#include "src/process/processor/Processor.h"

#include <string.h>

#include "src/lib/Type.h"

void
process::Processor::StreamData(air::string_view& node_name_view, uint32_t tid,
    const char* tname, node::NodeData* node_data,
    air::ProcessorType ptype, uint32_t new_time,
    uint32_t index_size, uint32_t filter_size)
{
    if (nullptr == node_data)
    {
        return;
    }
    time = new_time;

    lib::Data* air_data{nullptr};
    lib::AccData* acc_data{nullptr};
    uint64_t hash_value{0};
    for (uint32_t hash_index = 0; hash_index < index_size; hash_index++)
    {
        for (uint32_t filter_index = 0; filter_index < filter_size; filter_index++)
        {
            air_data = node_data->GetAirData(hash_index, filter_index);
            acc_data = node_data->GetAccData(hash_index, filter_index);
            if (nullptr != air_data && 0 != air_data->access && nullptr != acc_data)
            {
                hash_value = node_data->GetUserHashValue(hash_index);
                _ProcessData(air_data, acc_data);
                _JsonifyData(air_data, acc_data,
                    node_name_view, tid, tname, hash_value, filter_index);
                _InitData(air_data, acc_data);
            }
        }

        node_data->SwapBuffer(hash_index);
    }
}

void
process::Processor::StreamData(air::string_view& node_name_view,
    lib::AccLatencyData* data, uint32_t hash_index, uint32_t filter_index)
{
    if (nullptr == data)
    {
        return;
    }

    if (0 != data->sample_count || 0 != data->total_sample_count)
    {
        _ProcessData(nullptr, data);
        _JsonifyData(nullptr, data, node_name_view, 0, "", hash_index, filter_index);
        _InitData(nullptr, data);
    }
}
