
#include "mock_node_manager.h"
#include "src/target_detector/DetectCorHandler.h"
#include "src/target_detector/Detector.cpp"

class DetectCoRHandlerTest : public ::testing::Test
{
public:
    MockNodeManager* mock_node_manager{nullptr};
    detect::Detector* detector{nullptr};
    detect::DetectCoRHandler* detect_cor_handler{nullptr};

protected:
    DetectCoRHandlerTest()
    {
        mock_node_manager = new MockNodeManager{};
        detector = new detect::Detector{mock_node_manager};
        detect_cor_handler = new detect::DetectCoRHandler{detector};
    }
    ~DetectCoRHandlerTest()
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
    void
    SetUp() override
    {
    }
    void
    TearDown() override
    {
    }
};
