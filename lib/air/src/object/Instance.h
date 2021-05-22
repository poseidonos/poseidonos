
#ifndef AIR_INSTANCE_H
#define AIR_INSTANCE_H

#include "src/chain/ChainManager.h"
#include "src/collection/CollectionCorHandler.h"
#include "src/collection/CollectionManager.h"
#include "src/collection/CollectionObserver.h"
#include "src/collection/SwitchGear.h"
#include "src/collection/SwitchGearCorHandler.h"
#include "src/data_structure/NodeManager.h"
#include "src/input/In.h"
#include "src/input/InCorHandler.h"
#include "src/meta/GlobalMeta.h"
#include "src/meta/NodeMeta.h"
#include "src/output/OutputCorHandler.h"
#include "src/output/OutputManager.h"
#include "src/output/OutputObserver.h"
#include "src/policy/PolicyCorHandler.h"
#include "src/policy/PolicyObserver.h"
#include "src/policy/RuleManager.h"
#include "src/policy/Ruler.h"
#include "src/process/PreprocessCorHandler.h"
#include "src/process/Preprocessor.h"
#include "src/process/ProcessCorHandler.h"
#include "src/process/ProcessManager.h"
#include "src/stream/Stream.h"
#include "src/stream/StreamCorHandler.h"
#include "src/target_detector/DetectCorHandler.h"
#include "src/target_detector/Detector.h"
#include "src/thread/ThreadManager.h"

namespace air
{
class InstanceManager
{
public:
    InstanceManager(void);
    virtual ~InstanceManager(void);
    virtual int Initialize(uint32_t cpu_num);

    virtual int Finalize(void);
    virtual int Activate(void);
    virtual int Deactivate(void);

    virtual node::NodeManager*
    GetNodeManager(void)
    {
        return node_manager;
    }
    virtual collection::CollectionManager*
    GetCollectionManager(void)
    {
        return collection_manager;
    }

private:
    void _DeleteLibModule(void);
    void _DeleteMetaModule(void);
    void _DeletePolicyModule(void);
    void _DeleteProcessModule(void);
    void _DeleteDataStructureModule(void);
    void _DeleteOutModule(void);
    void _DeleteStreamModule(void);
    void _DeleteCollectionModule(void);
    void _DeleteChainModule(void);
    void _DeleteThreadModule(void);
    void _DeleteDetectorModule(void);

    meta::NodeMeta* node_meta{nullptr};
    meta::NodeMetaGetter* node_meta_getter{nullptr};
    meta::GlobalMeta* global_meta{nullptr};
    meta::GlobalMetaGetter* global_meta_getter{nullptr};

    chain::ChainManager* chain_manager{nullptr};

    collection::CollectionManager* collection_manager{nullptr};
    collection::Subject* collection_subject{nullptr};
    collection::Observer* collection_observer{nullptr};
    collection::CollectionCoRHandler* collection_cor_handler{nullptr};
    collection::SwitchGearCoRHandler* switch_gear_cor_handler{nullptr};
    collection::SwitchGear* switch_gear{nullptr};

    input::InCommand* in_command{nullptr};
    input::InCoRHandler* in_cor_handler{nullptr};
    input::Subject* in_subject{nullptr};

    policy::Subject* policy_subject{nullptr};
    policy::RuleManager* policy_rule_manager{nullptr};
    policy::PolicyCoRHandler* policy_cor_handler{nullptr};
    policy::Observer* policy_observer{nullptr};
    policy::Ruler* policy_ruler{nullptr};

    process::ProcessCoRHandler* process_cor_handler{nullptr};
    process::ProcessManager* process_manager{nullptr};
    process::PreprocessCoRHandler* preprocess_cor_handler{nullptr};
    process::Preprocessor* preprocessor{nullptr};

    node::NodeManager* node_manager{nullptr};

    output::OutCommand* out_command{nullptr};
    output::OutputCoRHandler* out_cor_handler{nullptr};
    output::Observer* out_observer{nullptr};
    output::OutputManager* out_manager{nullptr};

    stream::Stream* stream{nullptr};
    stream::StreamCoRHandler* stream_cor_handler{nullptr};

    thread::ThreadManager* thread_manager{nullptr};

    detect::DetectCoRHandler* detect_cor_handler{nullptr};
    detect::Detector* detector{nullptr};
};

} // namespace air

#endif // AIR_INSTANCE_H
