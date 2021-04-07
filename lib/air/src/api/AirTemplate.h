
#ifndef AIR_TEMPLATE_H
#define AIR_TEMPLATE_H

#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "src/collection/CollectionManager.h"
#include "src/config/ConfigInterface.h"
#include "src/lib/Casting.h"
#include "src/lib/Type.h"
#include "src/object/Instance.h"
#include "src/profile_data/node/NodeManager.h"

/*
#define AIR_INITIALIZE(...) \
    AIR<cfg::GetIntValue(config::ConfigType::DEFAULT, "AirBuild"),
true>::Initialize(__VA_ARGS__)
#define AIR_ACTIVATE() \
    AIR<cfg::GetIntValue(config::ConfigType::DEFAULT, "AirBuild"),
true>::Activate()
#define AIR_DEACTIVATE() \
    AIR<cfg::GetIntValue(config::ConfigType::DEFAULT, "AirBuild"),
true>::Deactivate()
#define AIR_FINALIZE() \
    AIR<cfg::GetIntValue(config::ConfigType::DEFAULT, "AirBuild"),
true>::Finalize()

#define AIRLOG(node, aid, value1, value2) \
    AIR<cfg::GetIntValue(config::ConfigType::DEFAULT, "AirBuild"), \
        cfg::GetIntValue(config::ConfigType::NODE, "NodeBuild", node)> \
    ::LogData<cfg::GetIndex(config::ConfigType::NODE, node)>(aid, value1,
value2)
*/

// Primary template
template<bool AirBuild, bool NodeBuild>
class AIR
{
public:
    static void Initialize(uint32_t cpu_num = 0);
    static void Activate(void);
    static void Deactivate(void);
    static void Finalize(void);

    template<uint32_t nid>
    static void LogData(uint32_t aid, uint32_t value1, uint64_t value2);

    static air::InstanceManager* instance_manager;
    static node::NodeManager* node_manager;
    static collection::CollectionManager* collection_manager;
    static thread_local node::ThreadArray* thread_array;
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

    template<uint32_t nid>
    static void
    LogData(uint32_t aid, uint32_t value1, uint64_t value2)
    {
        if ((nullptr == collection_manager) ||
            (false == collection_manager->IsLog(nid)))
        {
            return;
        }

        if (nullptr == thread_array && nullptr != node_manager)
        {
            uint32_t tid = syscall(SYS_gettid);
            thread_array = node_manager->GetThread(tid);
            node_manager->SetThreadName(tid);
        }
        if (nullptr != thread_array)
        {
            collection_manager->LogData(nid, aid, thread_array, value1, value2);
        }
    }

    static air::InstanceManager* instance_manager;
    static node::NodeManager* node_manager;
    static collection::CollectionManager* collection_manager;
    static thread_local node::ThreadArray* thread_array;
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

    template<uint32_t nid>
    static void
    LogData(uint32_t aid, uint32_t value1, uint64_t value2)
    {
    }

    static air::InstanceManager* instance_manager;
    static node::NodeManager* node_manager;
    static collection::CollectionManager* collection_manager;
    static thread_local node::ThreadArray* thread_array;
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
    template<uint32_t nid>
    static void
    LogData(uint32_t aid, uint32_t value1, uint64_t value2)
    {
    }
};

#endif // AIR_TEMPLATE_H
