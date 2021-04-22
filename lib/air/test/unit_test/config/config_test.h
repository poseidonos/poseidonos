
#include "src/config/ConfigParser.cpp"
#include "src/config/ConfigParser.h"

class ConfigTest : public ::testing::Test
{
public:
    config::Config* cfg{nullptr};

protected:
    ConfigTest()
    {
        cfg = new config::Config{};
    }
    ~ConfigTest()
    {
        if (nullptr != cfg)
        {
            delete cfg;
            cfg = nullptr;
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
