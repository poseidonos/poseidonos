
#include "src/output/Out.cpp"
#include "src/output/Out.h"

class OutTest : public ::testing::Test
{
public:
    output::OutCommand* out_command{nullptr};

protected:
    OutTest()
    {
        out_command = new output::OutCommand{};
    }
    ~OutTest() override
    {
        if (nullptr != out_command)
        {
            delete out_command;
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
