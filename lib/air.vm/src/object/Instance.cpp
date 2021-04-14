
#include "src/object/Instance.h"

#include "src/lib/Protocol.h"

air::InstanceManager::InstanceManager(void)
{
    node_meta = new meta::NodeMeta{};
    node_meta_getter = new meta::NodeMetaGetter{node_meta};
    global_meta = new meta::GlobalMeta{};
    global_meta_getter = new meta::GlobalMetaGetter{global_meta};

    in_subject = new input::Subject{};
    in_command = new input::InCommand{in_subject};
    in_cor_handler = new input::InCoRHandler{in_command};

    policy_ruler = new policy::Ruler{node_meta, global_meta};
    policy_subject = new policy::Subject{};
    policy_rule_manager = new policy::RuleManager{policy_ruler, policy_subject};
    policy_observer = new policy::Observer{policy_rule_manager};
    policy_cor_handler = new policy::PolicyCoRHandler{policy_observer};

    node_manager = new node::NodeManager{global_meta_getter, node_meta_getter};

    process_manager = new process::ProcessManager{global_meta_getter,
        node_meta_getter, node_manager};
    process_cor_handler = new process::ProcessCoRHandler{process_manager};
    preprocessor = new process::Preprocessor{node_meta_getter, global_meta_getter,
        node_manager};
    preprocess_cor_handler = new process::PreprocessCoRHandler{preprocessor};

    out_command = new output::OutCommand{};
    out_manager = new output::OutputManager{out_command};
    out_observer = new output::Observer{out_manager};
    out_cor_handler = new output::OutputCoRHandler{out_observer};

    stream = new stream::Stream{};
    stream_cor_handler = new stream::StreamCoRHandler{stream};

    collection_subject = new collection::Subject{};
    collection_manager = new collection::CollectionManager{
        global_meta_getter, node_meta_getter, node_manager, collection_subject};
    collection_observer = new collection::Observer{collection_manager};
    collection_cor_handler =
        new collection::CollectionCoRHandler{collection_observer};
    switch_gear = new collection::SwitchGear{node_meta_getter, global_meta_getter,
        node_manager};
    switch_gear_cor_handler = new collection::SwitchGearCoRHandler{switch_gear};

    chain_manager = new chain::ChainManager{global_meta};
    thread_manager = new thread::ThreadManager{chain_manager};

    detector = new detect::Detector{node_manager};
    detect_cor_handler = new detect::DetectCoRHandler{detector};
}

void
air::InstanceManager::_DeleteLibModule(void)
{
    if (nullptr != in_command)
    {
        delete in_command;
        in_command = nullptr;
    }
    if (nullptr != in_cor_handler)
    {
        delete in_cor_handler;
        in_cor_handler = nullptr;
    }
    if (nullptr != in_subject)
    {
        delete in_subject;
        in_subject = nullptr;
    }
}

void
air::InstanceManager::_DeleteMetaModule(void)
{
    if (nullptr != node_meta)
    {
        delete node_meta;
        node_meta = nullptr;
    }
    if (nullptr != node_meta_getter)
    {
        delete node_meta_getter;
        node_meta_getter = nullptr;
    }
    if (nullptr != global_meta)
    {
        delete global_meta;
        global_meta = nullptr;
    }
    if (nullptr != global_meta_getter)
    {
        delete global_meta_getter;
        global_meta_getter = nullptr;
    }
}

void
air::InstanceManager::_DeletePolicyModule(void)
{
    if (nullptr != policy_subject)
    {
        delete policy_subject;
        policy_subject = nullptr;
    }
    if (nullptr != policy_rule_manager)
    {
        delete policy_rule_manager;
        policy_rule_manager = nullptr;
    }
    if (nullptr != policy_cor_handler)
    {
        delete policy_cor_handler;
        policy_cor_handler = nullptr;
    }
    if (nullptr != policy_observer)
    {
        delete policy_observer;
        policy_observer = nullptr;
    }
    if (nullptr != policy_ruler)
    {
        delete policy_ruler;
        policy_ruler = nullptr;
    }
}

void
air::InstanceManager::_DeleteProcessModule(void)
{
    if (nullptr != process_manager)
    {
        delete process_manager;
        process_manager = nullptr;
    }
    if (nullptr != process_cor_handler)
    {
        delete process_cor_handler;
        process_cor_handler = nullptr;
    }
    if (nullptr != preprocess_cor_handler)
    {
        delete preprocess_cor_handler;
        preprocess_cor_handler = nullptr;
    }
    if (nullptr != preprocessor)
    {
        delete preprocessor;
        preprocessor = nullptr;
    }
}

void
air::InstanceManager::_DeleteProfileDataModule(void)
{
    if (nullptr != node_manager)
    {
        delete node_manager;
        node_manager = nullptr;
    }
}

void
air::InstanceManager::_DeleteOutModule(void)
{
    if (nullptr != out_command)
    {
        delete out_command;
        out_command = nullptr;
    }
    if (nullptr != out_manager)
    {
        delete out_manager;
        out_manager = nullptr;
    }
    if (nullptr != out_observer)
    {
        delete out_observer;
        out_observer = nullptr;
    }
    if (nullptr != out_cor_handler)
    {
        delete out_cor_handler;
        out_cor_handler = nullptr;
    }
}

void
air::InstanceManager::_DeleteStreamModule(void)
{
    if (nullptr != stream)
    {
        delete stream;
        stream = nullptr;
    }
    if (nullptr != stream_cor_handler)
    {
        delete stream_cor_handler;
        stream_cor_handler = nullptr;
    }
}

void
air::InstanceManager::_DeleteCollectionModule(void)
{
    if (nullptr != collection_manager)
    {
        delete collection_manager;
        collection_manager = nullptr;
    }
    if (nullptr != collection_subject)
    {
        delete collection_subject;
        collection_subject = nullptr;
    }
    if (nullptr != collection_observer)
    {
        delete collection_observer;
        collection_observer = nullptr;
    }
    if (nullptr != collection_cor_handler)
    {
        delete collection_cor_handler;
        collection_cor_handler = nullptr;
    }
    if (nullptr != switch_gear_cor_handler)
    {
        delete switch_gear_cor_handler;
        switch_gear_cor_handler = nullptr;
    }
    if (nullptr != switch_gear)
    {
        delete switch_gear;
        switch_gear = nullptr;
    }
}

void
air::InstanceManager::_DeleteChainModule(void)
{
    if (nullptr != chain_manager)
    {
        delete chain_manager;
        chain_manager = nullptr;
    }
}

void
air::InstanceManager::_DeleteThreadModule(void)
{
    if (nullptr != thread_manager)
    {
        delete thread_manager;
        thread_manager = nullptr;
    }
}

void
air::InstanceManager::_DeleteDetectorModule(void)
{
    if (nullptr != detector)
    {
        delete detector;
        detector = nullptr;
    }
    if (nullptr != detect_cor_handler)
    {
        delete detect_cor_handler;
        detect_cor_handler = nullptr;
    }
}

air::InstanceManager::~InstanceManager(void)
{
    _DeleteProcessModule();
    _DeleteProfileDataModule();
    _DeleteOutModule();
    _DeleteStreamModule();
    _DeleteCollectionModule();
    _DeleteChainModule();
    _DeleteThreadModule();
    _DeleteDetectorModule();
    _DeletePolicyModule();
    _DeleteMetaModule();
    _DeleteLibModule();
}

int
air::InstanceManager::Initialize(uint32_t cpu_num)
{
    // Step 1. Config Setting
    policy_rule_manager->SetNodeMetaConfig(node_meta->Meta());
    policy_rule_manager->SetGlobalConfig();

    // Step 2. Subject Referencing Observer
    in_subject->Attach(policy_observer, to_dtype(pi::InSubject::TO_POLICY));
    in_subject->Attach(out_observer, to_dtype(pi::InSubject::TO_OUTPUT));
    policy_subject->Attach(collection_observer,
        to_dtype(pi::PolicySubject::TO_COLLECTION));
    policy_subject->Attach(out_observer, to_dtype(pi::PolicySubject::TO_OUTPUT));
    collection_subject->Attach(out_observer,
        to_dtype(pi::CollectionSubject::TO_OUTPUT));

    // Step 3. ChainManager Referencing CoRHandler
    chain_manager->AttachChain(in_cor_handler, to_dtype(pi::ChainHandler::INPUT));
    chain_manager->AttachChain(policy_cor_handler,
        to_dtype(pi::ChainHandler::POLICY));
    chain_manager->AttachChain(collection_cor_handler,
        to_dtype(pi::ChainHandler::COLLECTION));
    chain_manager->AttachChain(out_cor_handler,
        to_dtype(pi::ChainHandler::OUTPUT));
    chain_manager->AttachChain(process_cor_handler,
        to_dtype(pi::ChainHandler::PROCESS));
    chain_manager->AttachChain(stream_cor_handler,
        to_dtype(pi::ChainHandler::STREAM));
    chain_manager->AttachChain(switch_gear_cor_handler,
        to_dtype(pi::ChainHandler::SWITCHGEAR));
    chain_manager->AttachChain(preprocess_cor_handler,
        to_dtype(pi::ChainHandler::PREPROCESS));
    chain_manager->AttachChain(detect_cor_handler,
        to_dtype(pi::ChainHandler::DETECT));

    // Step 4. NodeManager Initializing (using Config)
    node_manager->Init();

    // Step 5. CollectionManager Initializing (using Config)
    collection_manager->Init();

    // Step 6. ProcessManager Initializing (using Config)
    process_manager->Init();

    // Step 7. ChainManager Initializing (using Config)
    chain_manager->Init();

    // Step 8. Set CPU (using Config)
    policy_ruler->SetCpuNum(cpu_num);
    chain_manager->SetCpuSet(cpu_num);
    thread_manager->SetCpuSet(cpu_num);

    return 0;
}

int
air::InstanceManager::Finalize(void)
{
    return 0;
}

int
air::InstanceManager::Activate(void)
{
    chain_manager->StartThread();
    thread_manager->StartThread();

    return 0;
}

int
air::InstanceManager::Deactivate(void)
{
    chain_manager->JoinThread();
    thread_manager->JoinThread();

    return 0;
}
