
#include "src/lib/Hash.cpp"
#include "src/profile_data/node/NodeThread.cpp"
#include "src/profile_data/node/NodeThread.h"

class FakeThread : public node::Thread
{
public:
    FakeThread(air::ProcessorType new_type, uint32_t new_aid_size)
    {
        if (ptype != new_type)
        {
            for (int i = 0; i < 3; i++)
            {
                delete air_data[i];
                air_data[i] = nullptr;
                delete acc_data[i];
                acc_data[i] = nullptr;
            }
        }
        else
            return;

        SetIsLogging(true);
        ptype = new_type;
        switch (ptype)
        {
            case air::ProcessorType::PERFORMANCE:
            {
                for (int i = 0; i < 3; i++)
                {
                    air_data[i] = new lib::PerformanceData;
                    acc_data[i] = new lib::AccPerformanceData;
                }
                air_data[0]->access = true;
                lib::PerformanceData* perf_data = static_cast<lib::PerformanceData*>(air_data[0]);
                perf_data->packet_cnt[4096] = 1234;
                air_data[1]->access = true;
                break;
            }
            case air::ProcessorType::LATENCY:
            {
                for (int i = 0; i < 3; i++)
                {
                    air_data[i] = new lib::LatencyData;
                    acc_data[i] = new lib::AccLatencyData;
                }
                air_data[0]->access = true;
                air_data[1]->access = true;

                break;
            }
            case air::ProcessorType::QUEUE:
            {
                for (int i = 0; i < 3; i++)
                {
                    air_data[i] = new lib::QueueData;
                    acc_data[i] = new lib::AccQueueData;
                }
                air_data[0]->access = true;
                air_data[1]->access = true;
                ((lib::QueueData*)air_data[1])->num_req = 10;
                ((lib::QueueData*)air_data[1])->depth_period_max = 5;
                break;
            }
            case air::ProcessorType::UTILIZATION:
            {
                for (int i = 0; i < 3; i++)
                {
                    air_data[i] = new lib::UtilizationData;
                    acc_data[i] = new lib::AccUtilizationData;
                }
                ((lib::UtilizationData*)air_data[1])->usage[0] = 40;
                ((lib::UtilizationData*)air_data[1])->usage[1] = 15;
                ((lib::UtilizationData*)air_data[1])->sum = 55;

                air_data[0]->access = true;
                air_data[1]->access = true;
                break;
            }
            case air::ProcessorType::COUNT:
            {
                for (int i = 0; i < 3; i++)
                {
                    air_data[i] = new lib::CountData;
                    acc_data[i] = new lib::AccCountData;
                }
                ((lib::CountData*)air_data[1])->count[0] = 3454;
                ((lib::CountData*)air_data[1])->num_req[0] = 12;

                air_data[0]->access = true;
                air_data[1]->access = true;
                break;
            }
            default:
            {
                break;
            }
        }
        acc_data[0]->need_erase = 1;
    }

    virtual ~FakeThread()
    {
        for (int i = 0; i < 3; i++)
        {
            if (nullptr != air_data[i])
                delete air_data[i];
        }
    }

    lib::Data*
    GetAirData(uint32_t aid_idx)
    {
        return air_data[aid_idx];
    }

    lib::AccData*
    GetAccData(uint32_t aid_idx)
    {
        return acc_data[aid_idx];
    }

    uint64_t
    GetUserAidValue(uint32_t aid_idx)
    {
        return aid_idx;
    }

    void
    SwapBuffer(uint32_t aid_idx)
    {
        return;
    }

    void
    SetEnable()
    {
        air_data[0]->access = true;
        air_data[1]->access = true;
    }

private:
    lib::Data* air_data[100]{
        nullptr,
    };
    lib::AccData* acc_data[100]{
        nullptr,
    };
    air::ProcessorType ptype{air::ProcessorType::PROCESSORTYPE_NULL};
    bool is_logging{false};
};
