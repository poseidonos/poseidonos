
#include "src/config/ConfigChecker.cpp"
#include "src/config/ConfigChecker.h"

class ConfigCheckerTest : public ::testing::Test
{
public:
    config::ConfigChecker* cfg_checker{nullptr};

protected:
    ConfigCheckerTest()
    {
        cfg_checker = new config::ConfigChecker{};
    }
    ~ConfigCheckerTest()
    {
        if (nullptr != cfg_checker)
        {
            delete cfg_checker;
            cfg_checker = nullptr;
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
