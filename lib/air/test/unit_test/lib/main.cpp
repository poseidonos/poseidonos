
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "src/lib/Protocol.h"
#include "src/config/ConfigParser.cpp"

#include "casting_test.h"
#include "data_test.h"
#include "design_test.h"
#include "lock_test.h"
#include "msg_test.h"
#include "type_test.h"
#include "hash_test.h"
#include "json_test.h"

using ::testing::HasSubstr;

TEST_F(TypeTest, Node)
{
    EXPECT_EQ((uint32_t)cfg::GetIndex(config::ConfigType::NODE, "Q_COMPLETION"), node.nid);
    EXPECT_EQ(air::ProcessorType::QUEUE, node.processor_type);
    EXPECT_EQ(true, node.enable);
    EXPECT_EQ(1000U, node.sample_ratio);

    node.nid = (uint32_t)cfg::GetIndex(config::ConfigType::NODE, "PERF_BENCHMARK");
    EXPECT_EQ(0, cfg::GetName(config::ConfigType::NODE, node.nid).Compare("PERF_BENCHMARK"));
    node.nid = (uint32_t)cfg::GetIndex(config::ConfigType::NODE, "LAT_SUBMIT");
    EXPECT_EQ(0, cfg::GetName(config::ConfigType::NODE, node.nid).Compare("LAT_SUBMIT"));
    node.nid = (uint32_t)cfg::GetIndex(config::ConfigType::NODE, "LAT_PROCESS");
    EXPECT_EQ(0, cfg::GetName(config::ConfigType::NODE, node.nid).Compare("LAT_PROCESS"));
    node.nid = (uint32_t)cfg::GetIndex(config::ConfigType::NODE, "LAT_COMPLETE");
    EXPECT_EQ(0, cfg::GetName(config::ConfigType::NODE, node.nid).Compare("LAT_COMPLETE"));
    node.nid = (uint32_t)cfg::GetIndex(config::ConfigType::NODE, "Q_SUBMISSION");
    EXPECT_EQ(0, cfg::GetName(config::ConfigType::NODE, node.nid).Compare("Q_SUBMISSION"));
    node.nid = (uint32_t)cfg::GetIndex(config::ConfigType::NODE, "Q_COMPLETION");
    EXPECT_EQ(0, cfg::GetName(config::ConfigType::NODE, node.nid).Compare("Q_COMPLETION"));
}

TEST(Protocol, ValueCheck)
{
    uint32_t value32 {0x00000000};
    EXPECT_EQ(to_dtype(pi::Type1::INPUT_TO_POLICY), value32);

    value32 = 0x00010012;
    EXPECT_EQ(to_dtype(pi::Type2::INITIALIZE_NODE_WITH_RANGE), value32);

    uint16_t value16 {0x0001};
    EXPECT_EQ(to_dtype(pi::Type2_Upper::COLLECTION), value16);

    value16 = 0x0106;
    EXPECT_EQ(to_dtype(pi::Type2_Lower::SET_SAMPLING_RATE_ALL), value16);

    value32 = 5;
    EXPECT_EQ(to_dtype(pi::ChainHandler::STREAM), value32);

    const uint32_t max_value {pi::k_max_subject_size};
    EXPECT_GE(max_value, to_dtype(pi::InSubject::COUNT));
    EXPECT_GE(max_value, to_dtype(pi::PolicySubject::COUNT));
}

TEST_F(MsgTest, MsgEntry)
{
    uint32_t value {1};
    EXPECT_EQ(entry.type1, value);

    value = 2;
    EXPECT_EQ(entry.type2, value);

    value = 3;
    EXPECT_EQ(entry.value1, value);

    value = 4;
    EXPECT_EQ(entry.value2, value);
}

TEST_F(LockTest, Lock)
{
    lock->Lock();

    bool result = lock->TryLock();
    EXPECT_EQ(result, false);

    lock->Unlock();

    result = lock->TryLock();
    EXPECT_EQ(result, true);

    lock->Unlock();
}

TEST_F(DesignTest, Attach_Notify_Update)
{
    int result = subject->Attach(observer, 0);
    EXPECT_EQ(result, 0);

    result = subject->Attach(observer, 100);
    EXPECT_EQ(result, -3);

    result = subject->Notify(0, 1, 2, 3, 4, 0, 0, 0);
    EXPECT_EQ(result, 0);
}

TEST_F(DataTest, QueueData)
{
    bool b {false};
    uint64_t value {0xFF00EE00DD00CA00};
    EXPECT_EQ(b, queue_data->access);
    EXPECT_EQ(value, queue_data->sum_depth);
    EXPECT_LT(3.0f, queue_data->depth_period_avg);
}

TEST_F(DataTest, LatencyData)
{
    uint32_t value {0};
    EXPECT_EQ(value, lat_data->min);

    value = 0x0000FEAD;
    EXPECT_EQ(value, lat_data->max);
}

TEST_F(DataTest, PerformanceData)
{
    bool b {false};
    uint32_t value {0};
    EXPECT_EQ(b, perf_data->access);
    EXPECT_EQ(value, perf_data->idle_count);
    EXPECT_EQ(value, perf_data->iops_total);

    value = 100;
    EXPECT_EQ(value, perf_data->iops_read);
}

TEST_F(CastingTest, to_dtype)
{
    uint32_t num_u32 {2};
    EXPECT_EQ(num_u32, to_dtype(TypeUINT32_T::NUM_TWO));
    num_u32 = 0;
    EXPECT_EQ(num_u32, to_dtype(TypeUINT32_T::NUM_ZERO));
    EXPECT_NE(num_u32, to_dtype(TypeUINT32_T::NUM_ONE));
    EXPECT_LT(num_u32, to_dtype(TypeUINT32_T::NUM_ONE));

    uint64_t num_u64 {2};
    EXPECT_EQ(num_u64, to_dtype(TypeUINT64_T::NUM_TWO));
    num_u64 = 0;
    EXPECT_EQ(num_u64, to_dtype(TypeUINT64_T::NUM_ZERO));
    EXPECT_NE(num_u64, to_dtype(TypeUINT64_T::NUM_ONE));
    EXPECT_LT(num_u64, to_dtype(TypeUINT64_T::NUM_ONE));

    int num_i {2};
    EXPECT_EQ(num_i, to_dtype(TypeINT::NUM_POSITIVE_TWO));
    num_i = -1;
    EXPECT_EQ(num_i, to_dtype(TypeINT::NUM_NEGATIVE_ONE));
    EXPECT_NE(num_i, to_dtype(TypeINT::NUM_POSITIVE_ONE));
    EXPECT_GT(num_i, to_dtype(TypeINT::NUM_NEGATIVE_TWO));

    unsigned int num_ui {2};
    EXPECT_EQ(num_ui, to_dtype(TypeUINT::NUM_TWO));
    num_ui = 0;
    EXPECT_EQ(num_ui, to_dtype(TypeUINT::NUM_ZERO));
    EXPECT_NE(num_ui, to_dtype(TypeUINT::NUM_ONE));
    EXPECT_LT(num_ui, to_dtype(TypeUINT::NUM_ONE));

    bool b {false};
    EXPECT_EQ(b, to_dtype(TypeBOOL::BOOL_FALSE));
    b = true;
    EXPECT_EQ(b, to_dtype(TypeBOOL::BOOL_TRUE));
    EXPECT_NE(b, to_dtype(TypeBOOL::BOOL_FALSE));
}

TEST_F(HashTest, hash)
{
    uint32_t size = 5;
    // Insert
    uint32_t index_0 = hash_map->InsertHashNode(0x00A3FC00);
    uint32_t index_1 = hash_map->InsertHashNode(1);
    uint32_t index_2 = hash_map->InsertHashNode(77777);
    uint32_t index_3 = hash_map->InsertHashNode(-1);
    uint32_t index_4 = hash_map->InsertHashNode(-3333);
    uint32_t full = hash_map->InsertHashNode(2222);
    EXPECT_EQ(size, hash_map->GetHashSize());
    EXPECT_EQ(full, size);

    // Get hash key
    EXPECT_EQ((uint32_t)0x00A3FC00, hash_map->GetHashKey(index_0));
    EXPECT_EQ((uint32_t)(-1), hash_map->GetHashKey(index_3));
    EXPECT_EQ((uint32_t)(-3333), hash_map->GetHashKey(index_4));
    uint32_t new_index = hash_map->InsertHashNode(0x00A3FC0F);
    EXPECT_EQ(size, hash_map->GetHashSize());
    EXPECT_NE((uint32_t)0x00A3FC0F, hash_map->GetHashKey(new_index));

    // Delete 
    EXPECT_EQ(true, hash_map->DeleteHashNode(-3333));
    EXPECT_EQ(true, hash_map->DeleteHashNode(0x00A3FC00));
    EXPECT_EQ(false, hash_map->DeleteHashNode(0x00A3FC0F));
    EXPECT_EQ(size-2, hash_map->GetHashSize());
    EXPECT_NE((uint32_t)0x00A3FC00, hash_map->GetHashKey(index_0));
    EXPECT_NE((uint32_t)(-3333), hash_map->GetHashKey(index_4));
    
    // Get hash index
    EXPECT_EQ(size, hash_map->GetHashIndex(0x00A3FC00));
    EXPECT_EQ(size, hash_map->GetHashIndex(-3333));
    index_0 = hash_map->InsertHashNode(0x12341234);
    index_4 = hash_map->InsertHashNode(-77777);
    EXPECT_EQ(size, hash_map->GetHashSize());
    EXPECT_EQ(index_0, hash_map->GetHashIndex(0x12341234));
    EXPECT_EQ(index_4, hash_map->GetHashIndex(-77777));
    EXPECT_EQ(index_1, hash_map->GetHashIndex(1));
    EXPECT_EQ(index_2, hash_map->GetHashIndex(77777));
}

TEST_F(JsonTest, json_value_type_number)
{
    auto& obj = air::json("obj");

    int16_t int16_value = -1;
    obj["int16"] = {int16_value};
    int32_t int32_value = -2;
    obj["int32"] = {int32_value};
    int64_t int64_value = -3;
    obj["int64"] = {int64_value};
    
    uint16_t uint16_value = 1;
    obj["uint16"] = {uint16_value};
    uint32_t uint32_value = 2;
    obj["uint32"] = {uint32_value};
    uint64_t uint64_value = 3;
    obj["uint64"] = {uint64_value};
    
    float float_value = 100.77;
    obj["float"] = {float_value};
    double double_value = 1.23;
    obj["double"] = {double_value};
    
    
    for (auto i : air::range(obj))
    {
        switch(i.type) {
            case air::JSONtype::INT16: {
                void* vp = i.value;
                int16_t value = *((int16_t*)vp);
                EXPECT_EQ(-1, value);
                break;
            }
            case air::JSONtype::INT32: {
                void* vp = i.value;
                int32_t value = *((int32_t*)vp);
                EXPECT_EQ(-2, value);
                break;
            }
            case air::JSONtype::INT64: {
                void* vp = i.value;
                int64_t value = *((int64_t*)vp);
                EXPECT_EQ(-3, value);
                break;
            }
            case air::JSONtype::UINT16: {
                void* vp = i.value;
                uint16_t value = *((uint16_t*)vp);
                EXPECT_EQ((uint16_t)1, value);
                break;
            }
            case air::JSONtype::UINT32: {
                void* vp = i.value;
                uint32_t value = *((uint32_t*)vp);
                EXPECT_EQ((uint32_t)2, value);
                break;
            }
            case air::JSONtype::UINT64: {
                void* vp = i.value;
                uint64_t value = *((uint64_t*)vp);
                EXPECT_EQ((uint64_t)3, value);
                break;
            }
            case air::JSONtype::FLOAT: {
                void* vp = i.value;
                float value = *((float*)vp);
                EXPECT_EQ(true, ((100.7 < value) && (100.8 > value)));
                break;
            }
            case air::JSONtype::DOUBLE: {
                void* vp = i.value;
                double value = *((double*)vp);
                EXPECT_EQ(true, ((1.22 < value) && (1.24 > value)));
                break;
            }
            default : {
                std::cout << "type:" << (uint32_t)i.type << std::endl;
                break;
            }
        }
    }
    air::json_clear();
}

TEST_F(JsonTest, json_value_type_bool)
{
    auto& obj = air::json("obj");

    obj["bool_true"] = {true};
    obj["bool_false"] = {false};

    for (auto i : air::range(obj))
    {
        if (0 == i.key.compare("bool_true")) {
            void* vp = i.value;
            bool value = *((bool*)vp);
            EXPECT_EQ(true, value);
        } else if (0 == i.key.compare("bool_false")) {
            void* vp = i.value;
            bool value = *((bool*)vp);
            EXPECT_EQ(false, value);
        } else {
            std::cout << "unexpected value\n";
        }
    }
    air::json_clear();
}

TEST_F(JsonTest, json_value_type_string)
{
    auto& obj = air::json("obj");

    obj["charp"] = {"const char p"};
    obj["string"] = {std::string("stringstring")};

    for (auto i : air::range(obj))
    {
        if (0 == i.key.compare("charp")) {
            void* vp = i.value;
            std::string value = *((std::string*)vp);
            EXPECT_EQ(0, value.compare("const char p"));
        } else if (0 == i.key.compare("string")) {
            void* vp = i.value;
            std::string value = *((std::string*)vp);
            EXPECT_EQ(0, value.compare("stringstring"));
        } else {
            std::cout << "unexpected value\n";
        }
    }
    air::json_clear();
}

TEST_F(JsonTest, json_value_type_null)
{
    auto& obj = air::json("obj");

    obj["null"] = {nullptr};
    
    for (auto i : air::range(obj))
    {
        if (0 == i.key.compare("null")) {
            EXPECT_EQ(true, air::JSONtype::NULLVAL == i.type);
        } else {
            std::cout << "unexpected value\n";
        }
    }
    air::json_clear();
}

TEST_F(JsonTest, json_export)
{
    auto& obj = air::json("obj");

    obj["num1"] = {1, 2, 3};
    obj["null"] = {nullptr};

    std::ofstream write_file;
    write_file.open("json_export.txt");
    write_file << obj << std::endl;
    write_file.close();
    
    air::json_clear();

    std::ifstream read_file;
    read_file.open("json_export.txt");
    std::string read_line;
    std::getline(read_file, read_line);
    read_file.close();

    EXPECT_EQ(0, read_line.compare("{\"null\": null, \"num1\": [1, 2, 3]}"));
}

TEST_F(JsonTest, json_value_type_array)
{
    auto& obj = air::json("obj");

    obj["arr1"] += {1};
    obj["arr1"] += {2.2, 3.3};
    obj["arr1"] += {nullptr};

    obj["arr2"] = {"a", "b", "c"};
    obj["arr2"] += {std::string("def")};
    obj["arr2"] += {true, false};

    std::ofstream write_file;
    write_file.open("json_value_type_array.txt");
    write_file << obj << std::endl;
    write_file.close();
    
    air::json_clear();

    std::ifstream read_file;
    read_file.open("json_value_type_array.txt");
    std::string read_line;
    std::getline(read_file, read_line);
    read_file.close();

    EXPECT_EQ(0, read_line.compare("{\"arr1\": [1, 2.2, 3.3, null], \"arr2\": [\"a\", \"b\", \"c\", \"def\", true, false]}"));
}

TEST_F(JsonTest, json_value_type_object)
{
    auto& obj = air::json("obj");

    obj["sub_1"] = {air::json("sub_1")};
    auto& sub_2 = air::json("sub_2");
    sub_2["s2_item"];
    obj["sub_2"] = {sub_2};

    obj["obj_item"] = {"item1"};
    air::json("sub_1")["s1_item"] = {"itemS1"};
    sub_2["s2_item"] = {"itemS2"};

    std::ofstream write_file;
    write_file.open("json_value_type_object.txt");
    write_file << obj << std::endl;
    write_file.close();
    
    air::json_clear();

    std::ifstream read_file;
    read_file.open("json_value_type_object.txt");
    std::string read_line;
    std::getline(read_file, read_line);
    read_file.close();

    EXPECT_EQ(0, read_line.compare("{\"obj_item\": \"item1\", \"sub_1\": {}, \"sub_2\": {\"s2_item\": \"itemS2\"}}"));
}

TEST_F(JsonTest, json_print)
{
    auto& obj = air::json("obj");

    long long longlong_value = 1234;
    obj["longlong"] = {longlong_value};
    unsigned long long ulonglong_value = 1234;
    obj["ulonglong"] = {ulonglong_value};

    int16_t int16_value = -1;
    obj["int16"] = {int16_value};
    int32_t int32_value = -2;
    obj["int32"] = {int32_value};
    int64_t int64_value = -3;
    obj["int64"] = {int64_value};
    
    uint16_t uint16_value = 1;
    obj["uint16"] = {uint16_value};
    uint32_t uint32_value = 2;
    obj["uint32"] = {uint32_value};
    uint64_t uint64_value = 3;
    obj["uint64"] = {uint64_value};
    
    float float_value = 100.77;
    obj["float"] = {float_value};
    double double_value = 1.23;
    obj["double"] = {double_value};

    std::cout << obj << std::endl;

    air::json_clear();
}

TEST_F(JsonTest, json_range)
{
    auto& obj = air::json("obj");
    obj["int_arr"] = {1, 2, 3};
    obj["str"] = {"string"};
    obj["bool"] = {true};
    obj["double_arr"] += {4.44};
    auto& obj2 = air::json("obj2");
    obj2["str2"] = {"strstr"};
    obj["obj"] = {obj2};

    std::cout << obj << std::endl;
    for (auto i : air::range(obj))
    {
        std::cout << i.key << " : " << obj[i.key] << std::endl;
    }
    
    air::json_clear();
}

TEST_F(JsonTest, json_use_case_1)
{
    auto& aid0 = air::json("aid0");
    aid0["aid"] = {0};
    aid0["iops_read"] = {979265};
    aid0["iops_write"] = {103239};
    aid0["cnt_1"] = {"4096(sz)-1082504(cnt)"};
    auto& aid1 = air::json("aid1");
    aid1["aid"] = {1};
    aid1["iops_read"] = {301422};
    aid1["iops_write"] = {2144};
    aid1["cnt_1"] = {"4096(sz)-303466(cnt)"};
    auto& aid2 = air::json("aid2");
    aid2["aid"] = {2};
    aid2["iops_read"] = {222222};
    aid2["iops_write"] = {2222};
    aid2["cnt_1"] = {"4096(sz)-224444(cnt)"};
    auto& tid0 = air::json("tid0");
    tid0["tid"] = {30234};
    tid0["tname"] = {"Reactor01"};
    tid0["aid_arr"] += {aid0, aid1, aid2};
    auto& perf = air::json("perf");
    perf["id"] = {0};
    perf["name"] = {"PERF_VOL"};
    perf["tid_arr"] = {tid0};
    auto& air = air::json("air");
    air["timestamp"] = {15849345};
    air["status"] = {"normal"};
    air["interval"] = {1};
    air["node"] += {perf};

    std::ofstream write_file;
    write_file.open("json_use_case_1.txt");
    write_file << air << std::endl;
    write_file.close();
    
    air::json_clear();

    std::ifstream read_file;
    read_file.open("json_use_case_1.txt");
    std::string read_line;
    std::getline(read_file, read_line);
    read_file.close();

    EXPECT_EQ(0, read_line.compare("{\"interval\": 1, \"node\": [{\"id\": 0, \"name\": \"PERF_VOL\", \"tid_arr\": {\"aid_arr\": [{\"aid\": 0, \"cnt_1\": \"4096(sz)-1082504(cnt)\", \"iops_read\": 979265, \"iops_write\": 103239}, {\"aid\": 1, \"cnt_1\": \"4096(sz)-303466(cnt)\", \"iops_read\": 301422, \"iops_write\": 2144}, {\"aid\": 2, \"cnt_1\": \"4096(sz)-224444(cnt)\", \"iops_read\": 222222, \"iops_write\": 2222}], \"tid\": 30234, \"tname\": \"Reactor01\"}}], \"status\": \"normal\", \"timestamp\": 15849345}"));
}

TEST_F(JsonTest, json_use_case_2)
{
    air::json("obj") = {};
    air::json("obj")["timestamp"] = {101, 102, 103, 104};
    air::json("obj")["name"] += {"name1", "name2"};

    auto& obj2 = air::json("obj2");
    obj2["age"] = {24, 22};

    auto& obj = air::json("obj");
    obj2["gender"] = {"male", "female"};
    obj["gender"] += {obj2["gender"]};

    std::ofstream write_file;
    write_file.open("json_use_case_2.txt");
    write_file << obj << std::endl;
    write_file.close();
    
    air::json_clear();

    std::ifstream read_file;
    read_file.open("json_use_case_2.txt");
    std::string read_line;
    std::getline(read_file, read_line);
    read_file.close();

    EXPECT_EQ(0, read_line.compare("{\"gender\": [[\"male\", \"female\"]], \"name\": [\"name1\", \"name2\"], \"timestamp\": [101, 102, 103, 104]}"));
}

TEST_F(JsonTest, json_parse_case_1)
{
    air::json("obj") = {};
    air::json("obj")["timestamp"] = {101, 102, 103, 104};
    air::json("obj")["name"] += {"name1", "name2"};

    auto& obj2 = air::json("obj2");
    obj2["age"] = {24, 22};

    auto& obj = air::json("obj");
    obj2["gender"] = {"male", "female"};
    obj["gender"] += {obj2["gender"]};

    std::ofstream write_file;
    write_file.open("json_parse_case_1.txt");
    write_file << obj << std::endl;
    write_file.close();
    
    air::json_clear();

    std::ifstream read_file;
    read_file.open("json_parse_case_1.txt");
    std::string read_line;
    std::getline(read_file, read_line);
    read_file.close();

    auto& parsed_obj = air::parse("parsed_data", read_line);

    write_file.open("json_parse_case_1_parsed.txt");
    write_file << parsed_obj << std::endl;
    write_file.close();

    air::json_clear();

    read_file.open("json_parse_case_1_parsed.txt");
    std::string read_line_parsed;
    std::getline(read_file, read_line_parsed);
    read_file.close();

    EXPECT_EQ(0, read_line.compare(read_line_parsed));
}

TEST_F(JsonTest, json_parse_case_2)
{
    auto& obj = air::json("obj");
    auto& sub = air::json("sub");
    auto& subsub = air::json("subsub");

    subsub["heart"] = {"beating"};
    sub["subsub"] = {subsub};
    sub["time"] = {2342354};

    obj["name"] = {"unknown"};
    obj["sub"] = {sub};
    obj["x-lay-tested"] = {true};
    obj["y-lay-tested"] = {false};
    obj["zoo"] = {"like"};
    obj["null"] = {nullptr};
    obj["age"] = {34};
    obj["condition"] = {-3};
    obj["luck"] = {0.77};
    

    std::ofstream write_file;
    write_file.open("json_parse_case_2.txt");
    write_file << obj << std::endl;
    write_file.close();
    
    air::json_clear();

    std::ifstream read_file;
    read_file.open("json_parse_case_2.txt");
    std::string read_line;
    std::getline(read_file, read_line);
    read_file.close();

    auto& parsed_obj = air::parse("parsed_data", read_line);

    write_file.open("json_parse_case_2_parsed.txt");
    write_file << parsed_obj << std::endl;
    write_file.close();

    air::json_clear();

    read_file.open("json_parse_case_2_parsed.txt");
    std::string read_line_parsed;
    std::getline(read_file, read_line_parsed);
    read_file.close();

    EXPECT_EQ(0, read_line.compare(read_line_parsed));
}

TEST_F(JsonTest, json_parse_case_3)
{
    auto& obj = air::json("obj");
    auto& sub = air::json("sub");

    obj["arr1"] += {0.77};
    obj["arr1"] += {1, 2, 3};
    obj["arr1"] += {nullptr};
    obj["arr1"] += {true, false};
    obj["arr1"] += {-324.2};
    obj["arr1"] += {-44};
    sub["bool"] = {false};
    obj["arr1"] += {sub};
    

    std::ofstream write_file;
    write_file.open("json_parse_case_3.txt");
    write_file << obj << std::endl;
    write_file.close();
    
    air::json_clear();

    std::ifstream read_file;
    read_file.open("json_parse_case_3.txt");
    std::string read_line;
    std::getline(read_file, read_line);
    read_file.close();

    auto& parsed_obj = air::parse("parsed_data", read_line);

    write_file.open("json_parse_case_3_parsed.txt");
    write_file << parsed_obj << std::endl;
    write_file.close();

    air::json_clear();

    read_file.open("json_parse_case_3_parsed.txt");
    std::string read_line_parsed;
    std::getline(read_file, read_line_parsed);
    read_file.close();

    std::cout << read_line << std::endl;
    std::cout << read_line_parsed << std::endl;

    EXPECT_EQ(0, read_line.compare(read_line_parsed));
}

TEST_F(JsonTest, jsondoc_api_exception)
{
    auto& obj = air::json("obj");

    try {
        obj = {"invalid usage"};
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    try {
        obj += {1};
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    try {
        obj += {"appending const cp"};
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    try {
        obj += {air::json("obj2")};
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    try {
        uint8_t uint8_value {1};
        obj["uint8_t"] += {uint8_value};
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    auto& obj_whitespace = air::json("obj_whitespace");
    obj_whitespace.SetType(air::JSONtype::WHITESPACE);
    std::cout << obj_whitespace << std::endl;

    auto& obj_undefined = air::json("obj_undefined");
    obj_undefined.SetType(air::JSONtype::UNDEFINED);
    try {
        std::cout << obj_undefined << std::endl;
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    air::json_clear();
}

TEST_F(JsonTest, jsondoc_clear_exception)
{
    auto& obj = air::json("obj");
    long long longlong_value {1234};
    obj["longlong"] = {longlong_value};
    unsigned long long ulonglong_value {1234};
    obj["ulonglong"] = {ulonglong_value};
    obj = {};

    try {
        obj.DeleteValue(air::JSONtype::ARRAY, nullptr);
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    
    air::json_clear();
}

TEST_F(JsonTest, jsonprint_exception)
{
    try {
        air::PrintValue(std::cout, air::JSONtype::LONG, nullptr);
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}

TEST_F(JsonTest, jsonparser_exception)
{
    auto& obj = air::json("obj");
    
    static_cast<JSONparserTest&>(JSONparserTest::GetInstance()).TEST_AddJson(
        obj, "obj", air::JSONtype::NULLVAL, "null", "null");
    std::cout << obj << std::endl;

    try {
        static_cast<JSONparserTest&>(JSONparserTest::GetInstance()).TEST_AddJson(
            obj, "obj", air::JSONtype::LONG, "long", "1234");
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    auto& arr = air::json("arr");
    arr["int"] += {1, 2, 3};

    static_cast<JSONparserTest&>(JSONparserTest::GetInstance()).TEST_AddJsonArray(
        arr, "arr", 3, air::JSONtype::INT64, "int", "4");
    std::cout << arr << std::endl;

    try {
        static_cast<JSONparserTest&>(JSONparserTest::GetInstance()).TEST_AddJsonArray(
            arr, "arr", 4, air::JSONtype::LONG, "int", "5");
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    air::json_clear();
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
