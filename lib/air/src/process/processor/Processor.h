
#ifndef AIR_PROCESSOR_H
#define AIR_PROCESSOR_H

#include <string>

#include "src/data_structure/NodeData.h"
#include "src/lib/Data.h"
#include "src/lib/StringView.h"

namespace process
{
class Processor
{
public:
    virtual ~Processor(void)
    {
    }
    void StreamData(air::string_view& node_name_view, uint32_t tid, const char* tname,
        node::NodeData* node_data, air::ProcessorType ptype, uint32_t time,
        uint32_t index_size, uint32_t filter_size);
    void StreamData(air::string_view& node_name_view, lib::AccLatencyData* data,
        uint32_t hash_index, uint32_t filter_index);

protected:
    uint32_t time{1};

private:
    virtual void _ProcessData(lib::Data* air_data, lib::AccData* acc_data) = 0;
    virtual void _JsonifyData(lib::Data* air_data, lib::AccData* acc_data,
        air::string_view& node_name_view, uint32_t tid, const char* tname,
        uint64_t hash_value, uint32_t filter_index) = 0;
    virtual void _InitData(lib::Data* air_data, lib::AccData* acc_data) = 0;
};

} // namespace process

#endif // AIR_PROCESSOR_H
