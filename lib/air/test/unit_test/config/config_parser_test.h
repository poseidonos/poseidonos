
#include "src/config/ConfigParser.h"

class ConfigParserTest : public ::testing::Test
{
public:
    config::ConfigParser* cfg{nullptr};

protected:
    ConfigParserTest()
    {
        cfg = new config::ConfigParser{};
    }
    ~ConfigParserTest()
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
