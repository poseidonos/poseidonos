
#include "src/object/Instance.h"
#include "src/object/Instance.cpp"
#include "src/meta/NodeMeta.h"
#include "src/meta/GlobalMeta.h"
#include "src/chain/ChainManager.h"
#include "src/chain/ChainManager.cpp"
#include "src/collection/CollectionManager.h"
#include "src/collection/CollectionManager.cpp"
#include "src/collection/CollectionCorHandler.h"
#include "src/collection/SwitchGear.h"
#include "src/collection/SwitchGear.cpp"
#include "src/collection/SwitchGearCorHandler.h"
#include "src/collection/Collector.h"
#include "src/collection/Collector.cpp"
#include "src/collection/Writer.h"
#include "src/collection/Writer.cpp"
#include "src/collection/CollectionObserver.h"
#include "src/collection/CollectionObserver.cpp"
#include "src/input/In.h"
#include "src/input/In.cpp"
#include "src/input/InCorHandler.h"
#include "src/policy/PolicyCorHandler.h"
#include "src/policy/RuleManager.h"
#include "src/policy/RuleManager.cpp"
#include "src/policy/Ruler.h"
#include "src/policy/Ruler.cpp"
#include "src/policy/PolicyObserver.h"
#include "src/policy/PolicyObserver.cpp"
#include "src/process/ProcessCorHandler.h"
#include "src/process/PreprocessCorHandler.h"
#include "src/process/Preprocessor.h"
#include "src/process/Preprocessor.cpp"
#include "src/process/ProcessManager.h"
#include "src/process/ProcessManager.cpp"
#include "src/process/TimingDistributor.h"
#include "src/process/TimingDistributor.cpp"
#include "src/process/Processor.h"
#include "src/process/Processor.cpp"
#include "src/profile_data/node/NodeManager.h"
#include "src/profile_data/node/NodeManager.cpp"
#include "src/profile_data/node/NodeThread.h"
#include "src/profile_data/node/NodeThread.cpp"
#include "src/output/Out.h"
#include "src/output/Out.cpp"
#include "src/output/OutputManager.h"
#include "src/output/OutputManager.cpp"
#include "src/output/OutputCorHandler.h"
#include "src/output/OutputObserver.h"
#include "src/output/OutputObserver.cpp"
#include "src/stream/Stream.h"
#include "src/stream/Stream.cpp"
#include "src/stream/StreamCorHandler.h"
#include "src/thread/ThreadManager.h"
#include "src/thread/ThreadManager.cpp"
#include "src/thread/Thread.h"
#include "src/thread/Thread.cpp"
#include "src/target_detector/DetectCorHandler.h"
#include "src/target_detector/Detector.h"
#include "src/target_detector/Detector.cpp"
#include "src/lib/Design.h"
#include "src/lib/Design.cpp"
#include "src/lib/Hash.h"
#include "src/lib/Hash.cpp"

class InstanceTest : public ::testing::Test
{
public:
    air::InstanceManager* instance_manager {nullptr};
protected:
    InstanceTest() {
        instance_manager = new air::InstanceManager();
    }

    ~InstanceTest() {
        if (nullptr != instance_manager) {
            delete instance_manager;
            instance_manager = nullptr;
        }
    }
    void SetUp() override {

    }
    void TearDown() override {

    }
};
