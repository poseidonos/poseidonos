#include "src/lib/json/Json.h"

class JSONparserTest : public air::JSONparser
{
public:
    void
    TEST_AddJson(air::JSONdoc& doc, std::string doc_name,
        air::JSONtype type, std::string key, std::string value)
    {
        _AddJson(doc, doc_name, type, key, value);
    }
    void
    TEST_AddJsonArray(air::JSONdoc& doc, std::string doc_name,
        std::size_t value_count, air::JSONtype type, std::string key,
        std::string value)
    {
        _AddJsonArray(doc, doc_name, value_count, type, key, value);
    }
};

class JsonTest : public ::testing::Test
{
protected:
    JsonTest()
    {
    }
    ~JsonTest() override
    {
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