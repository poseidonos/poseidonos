
#include "src/lib/Msg.h"

class MsgTest : public ::testing::Test
{
public:
    lib::MsgEntry entry{0};

protected:
    MsgTest()
    {
    }
    ~MsgTest() override
    {
    }
    void
    SetUp() override
    {
        entry.type1 = 1;
        entry.type2 = 2;
        entry.value1 = 3;
        entry.value2 = 4;
    }
    void
    TearDown() override
    {
    }
};
