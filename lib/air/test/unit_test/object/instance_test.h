
#include "src/chain/ChainManager.cpp"
#include "src/chain/ChainManager.h"
#include "src/collection/CollectionCorHandler.h"
#include "src/collection/CollectionManager.cpp"
#include "src/collection/CollectionManager.h"
#include "src/collection/CollectionObserver.cpp"
#include "src/collection/CollectionObserver.h"
#include "src/collection/Collector.cpp"
#include "src/collection/Collector.h"
#include "src/collection/SwitchGear.cpp"
#include "src/collection/SwitchGear.h"
#include "src/collection/SwitchGearCorHandler.h"
#include "src/collection/writer/CountWriter.cpp"
#include "src/collection/writer/CountWriter.h"
#include "src/collection/writer/LatencyWriter.cpp"
#include "src/collection/writer/LatencyWriter.h"
#include "src/collection/writer/PerformanceWriter.cpp"
#include "src/collection/writer/PerformanceWriter.h"
#include "src/collection/writer/QueueWriter.cpp"
#include "src/collection/writer/QueueWriter.h"
#include "src/collection/writer/UtilizationWriter.cpp"
#include "src/collection/writer/UtilizationWriter.h"
#include "src/collection/writer/Writer.cpp"
#include "src/collection/writer/Writer.h"
#include "src/data_structure/NodeData.cpp"
#include "src/data_structure/NodeData.h"
#include "src/data_structure/NodeManager.cpp"
#include "src/data_structure/NodeManager.h"
#include "src/input/In.cpp"
#include "src/input/In.h"
#include "src/input/InCorHandler.h"
#include "src/lib/Design.cpp"
#include "src/lib/Design.h"
#include "src/lib/Hash.cpp"
#include "src/lib/Hash.h"
#include "src/meta/GlobalMeta.h"
#include "src/meta/NodeMeta.h"
#include "src/object/Instance.cpp"
#include "src/object/Instance.h"
#include "src/output/Out.cpp"
#include "src/output/Out.h"
#include "src/output/OutputCorHandler.h"
#include "src/output/OutputManager.cpp"
#include "src/output/OutputManager.h"
#include "src/output/OutputObserver.cpp"
#include "src/output/OutputObserver.h"
#include "src/policy/PolicyCorHandler.h"
#include "src/policy/PolicyObserver.cpp"
#include "src/policy/PolicyObserver.h"
#include "src/policy/RuleManager.cpp"
#include "src/policy/RuleManager.h"
#include "src/policy/Ruler.cpp"
#include "src/policy/Ruler.h"
#include "src/process/PreprocessCorHandler.h"
#include "src/process/Preprocessor.cpp"
#include "src/process/Preprocessor.h"
#include "src/process/ProcessCorHandler.h"
#include "src/process/ProcessManager.cpp"
#include "src/process/ProcessManager.h"
#include "src/process/TimingDistributor.cpp"
#include "src/process/TimingDistributor.h"
#include "src/process/processor/CountProcessor.cpp"
#include "src/process/processor/CountProcessor.h"
#include "src/process/processor/LatencyProcessor.cpp"
#include "src/process/processor/LatencyProcessor.h"
#include "src/process/processor/PerformanceProcessor.cpp"
#include "src/process/processor/PerformanceProcessor.h"
#include "src/process/processor/Processor.cpp"
#include "src/process/processor/Processor.h"
#include "src/process/processor/QueueProcessor.cpp"
#include "src/process/processor/QueueProcessor.h"
#include "src/process/processor/UtilizationProcessor.cpp"
#include "src/process/processor/UtilizationProcessor.h"
#include "src/stream/Stream.cpp"
#include "src/stream/Stream.h"
#include "src/stream/StreamCorHandler.h"
#include "src/target_detector/DetectCorHandler.h"
#include "src/target_detector/Detector.cpp"
#include "src/target_detector/Detector.h"
#include "src/thread/Thread.cpp"
#include "src/thread/Thread.h"
#include "src/thread/ThreadManager.cpp"
#include "src/thread/ThreadManager.h"

class InstanceTest : public ::testing::Test
{
public:
    air::InstanceManager* instance_manager{nullptr};

protected:
    InstanceTest()
    {
        instance_manager = new air::InstanceManager();
    }

    ~InstanceTest()
    {
        if (nullptr != instance_manager)
        {
            delete instance_manager;
            instance_manager = nullptr;
        }
    }
    void
    SetUp() override
    {
    }
    void
    TearDown() override
    {
    }
};
