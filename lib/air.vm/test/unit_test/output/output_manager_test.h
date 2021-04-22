
#include "fake_out.h"
#include "src/output/OutputManager.cpp"
#include "src/output/OutputManager.h"

class OutputManagerTest : public ::testing::Test
{
public:
    output::OutputManager* output_manager{nullptr};
    FakeOut* fake_out{nullptr};

protected:
    OutputManagerTest()
    {
        fake_out = new FakeOut{};
        output_manager = new output::OutputManager{fake_out};
    }
    ~OutputManagerTest() override
    {
        if (nullptr != fake_out)
        {
            delete fake_out;
        }
        if (nullptr != output_manager)
        {
            delete output_manager;
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
