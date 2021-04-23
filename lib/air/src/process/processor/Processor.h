
#ifndef AIR_PROCESSOR_H
#define AIR_PROCESSOR_H

#include <string>

#include "src/lib/Data.h"
#include "src/profile_data/node/NodeThread.h"

namespace process
{
class Processor
{
public:
    virtual ~Processor(void)
    {
    }
    bool StreamData(std::string node_name, uint32_t tid, const char* tname,
        node::Thread* thread, air::ProcessorType ptype, uint32_t time,
        uint32_t max_aid_size);
    bool StreamData(std::string node_name, lib::AccLatencyData* data,
        uint32_t aid);

protected:
    uint32_t time{1};

private:
    virtual bool _ProcessData(lib::Data* air_data, lib::AccData* acc_data) = 0;
    virtual void _InitData(lib::Data* air_data, lib::AccData* acc_data) = 0;
};

} // namespace process

#endif // AIR_PROCESSOR_H
