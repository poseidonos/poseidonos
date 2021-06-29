
#include "src/lib/Hash.cpp"
#include "src/lib/Hash.h"

class HashTest : public ::testing::Test
{
public:
    air::HashMap<uint32_t>* hash_map{nullptr};

protected:
    HashTest()
    {
        hash_map = new air::HashMap<uint32_t>{5};
    }
    ~HashTest() override
    {
        if (nullptr != hash_map)
        {
            delete hash_map;
            hash_map = nullptr;
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
