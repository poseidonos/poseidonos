
#ifndef AIR_H
#define AIR_H

#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "src/collection/CollectionManager.h"
#include "src/config/ConfigInterface.h"
#include "src/data_structure/NodeManager.h"
#include "src/lib/Casting.h"
#include "src/lib/Type.h"
#include "src/object/Instance.h"

#define air_initialize(...) \
    AIR<cfg::GetIntValue(config::ParagraphType::DEFAULT, "AirBuild"), true>::Initialize(__VA_ARGS__)
#define air_activate() \
    AIR<cfg::GetIntValue(config::ParagraphType::DEFAULT, "AirBuild"), true>::Activate()
#define air_deactivate() \
    AIR<cfg::GetIntValue(config::ParagraphType::DEFAULT, "AirBuild"), true>::Deactivate()
#define air_finalize() \
    AIR<cfg::GetIntValue(config::ParagraphType::DEFAULT, "AirBuild"), true>::Finalize()

#define airlog(node_name, filter_item, node_index, data)                                        \
    AIR<cfg::GetIntValue(config::ParagraphType::DEFAULT, "AirBuild"),                           \
        cfg::GetIntValue(config::ParagraphType::NODE, "Build",                                  \
            node_name)>::LogData<cfg::GetSentenceIndex(config::ParagraphType::NODE, node_name), \
        cfg::GetIntValue(config::ParagraphType::FILTER, "Item",                                 \
            cfg::GetStrValue(config::ParagraphType::NODE, "Filter", node_name), filter_item)>(node_index, data)

// Primary template
template<bool AirBuild, bool NodeBuild>
class AIR
{
public:
    static void Initialize(uint32_t cpu_num = 0);
    static void Activate(void);
    static void Deactivate(void);
    static void Finalize(void);

    template<int32_t node_id, int32_t filter_index>
    static void LogData(uint64_t node_index, uint64_t value);
    static void LogData(uint32_t node_id, uint32_t filter_index, uint64_t node_index, uint64_t value);

    static air::InstanceManager* instance_manager;
    static node::NodeManager* node_manager;
    static collection::CollectionManager* collection_manager;
    static thread_local node::NodeDataArray* node_data_array;
};

// AIR build : true && Node build : true
template<>
class AIR<true, true>
{
public:
    static void
    Activate(void)
    {
        if (nullptr != instance_manager)
        {
            instance_manager->Activate();
        }
    }
    static void
    Finalize(void)
    {
        if (nullptr != instance_manager)
        {
            instance_manager->Finalize();
            delete instance_manager;
            instance_manager = nullptr;
        }
    }
    static void
    Deactivate(void)
    {
        if (nullptr != instance_manager)
        {
            instance_manager->Deactivate();
        }
    }
    static void
    Initialize(uint32_t cpu_num = 0)
    {
        instance_manager = new air::InstanceManager();
        instance_manager->Initialize(cpu_num);
        node_manager = instance_manager->GetNodeManager();
        collection_manager = instance_manager->GetCollectionManager();
    }

    template<int32_t node_id, int32_t filter_index>
    static void
    LogData(uint64_t node_index, uint64_t value)
    {
        static_assert(-1 != node_id, "Invalid Node");
        static_assert(-1 != filter_index, "Invalid Filter Item");

        if ((nullptr == collection_manager) ||
            (false == collection_manager->IsLog(node_id)))
        {
            return;
        }

        if (nullptr != node_data_array)
        {
            collection_manager->LogData(node_id, filter_index, node_data_array, node_index, value);
        }
        else if (nullptr != node_manager)
        {
            uint32_t tid = syscall(SYS_gettid);
            node_data_array = node_manager->GetNodeDataArray(tid);
            node_manager->SetNodeDataArrayName(tid);
        }
    }

    static void
    LogData(uint32_t node_id, uint32_t filter_index, uint64_t node_index, uint64_t value)
    {
        if ((nullptr == collection_manager) ||
            (false == collection_manager->IsLog(node_id)))
        {
            return;
        }

        if (nullptr == node_data_array && nullptr != node_manager)
        {
            uint32_t tid = syscall(SYS_gettid);
            node_data_array = node_manager->GetNodeDataArray(tid);
            node_manager->SetNodeDataArrayName(tid);
        }
        if (nullptr != node_data_array)
        {
            collection_manager->LogData(node_id, filter_index, node_data_array, node_index, value);
        }
    }

    static air::InstanceManager* instance_manager;
    static node::NodeManager* node_manager;
    static collection::CollectionManager* collection_manager;
    static thread_local node::NodeDataArray* node_data_array;
};

// AIR build : false && Node build : true
template<>
class AIR<true, false>
{
public:
    static void
    Initialize(uint32_t cpu_num = 0)
    {
        instance_manager = new air::InstanceManager();
        instance_manager->Initialize(cpu_num);
        node_manager = instance_manager->GetNodeManager();
        collection_manager = instance_manager->GetCollectionManager();
    }
    static void
    Activate(void)
    {
        if (nullptr != instance_manager)
        {
            instance_manager->Activate();
        }
    }
    static void
    Deactivate(void)
    {
        if (nullptr != instance_manager)
        {
            instance_manager->Deactivate();
        }
    }
    static void
    Finalize(void)
    {
        if (nullptr != instance_manager)
        {
            instance_manager->Finalize();
            delete instance_manager;
            instance_manager = nullptr;
        }
    }

    template<int32_t node_id, int32_t filter_index>
    static void
    LogData(uint64_t node_index, uint64_t value)
    {
    }
    static void
    LogData(uint32_t node_id, uint32_t filter_index, uint64_t node_index, uint64_t value)
    {
    }

    static air::InstanceManager* instance_manager;
    static node::NodeManager* node_manager;
    static collection::CollectionManager* collection_manager;
    static thread_local node::NodeDataArray* node_data_array;
};

// AIR build : false
template<bool NodeBuild>
class AIR<false, NodeBuild>
{
public:
    static void
    Initialize(uint32_t cpu_num = 0)
    {
    }
    static void
    Activate(void)
    {
    }
    static void
    Deactivate(void)
    {
    }
    static void
    Finalize(void)
    {
    }
    template<int32_t node_id, int32_t filter_index>
    static void
    LogData(uint64_t node_index, uint64_t value)
    {
    }
    static void
    LogData(uint32_t node_id, uint32_t filter_index, uint64_t node_index, uint64_t value)
    {
    }
};

#endif // AIR_H
