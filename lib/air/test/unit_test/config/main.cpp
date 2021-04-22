
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include <iostream>

#include "config_checker_test.h"
#include "config_interface_test.h"
#include "config_test.h"

TEST_F(ConfigTest, GetEntryFromStrArr)
{
    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::DEFAULT).compare("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));

    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::GROUP, 1).compare("GroupName: POS_JOURNAL ,\nNodeBuild: True"));
    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::GROUP, 3).compare("GroupName: POS_META, NodeBuild: False, NodeRun: On, SamplingRatio: 100"));
    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::GROUP, 5).compare(""));

    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::NODE, 1).compare("NodeName: PERF_VOLUME, Type: PERFORMANCE, NodeBuild: True, GroupName: POS"));
    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::NODE, 4).compare("NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: write_bw>100MB/s, Delegation: HandleLowBW, GroupName: POS_JOURNAL"));
    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::NODE, 9).compare("NodeName: LAT_CP, Type: LATENCY, NodeBuild: False, NodeRun: Off, Condition: 4nine>1sec, Delegation: HandleTailLatency, GroupName: POS"));
    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::NODE, 10).compare("NodeName: Q_SUBMIT, Type: QUEUE, GroupName: POS"));
    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::NODE, 13).compare("NodeName: Q_SCHEDULING\n, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 1000,\n          Condition: depth<200, GroupName: POS"));
    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::GROUP, 99).compare(""));
}

TEST_F(ConfigTest, GetIndexFromStrArr)
{
    EXPECT_EQ(0, cfg->GetIndexFromStrArr(config::ConfigType::DEFAULT));

    EXPECT_EQ(0, cfg->GetIndexFromStrArr(config::ConfigType::GROUP, "POS"));
    EXPECT_EQ(1, cfg->GetIndexFromStrArr(config::ConfigType::GROUP, "POS_JOURNAL"));
    EXPECT_EQ(2, cfg->GetIndexFromStrArr(config::ConfigType::GROUP, "POS_IO"));
    EXPECT_EQ(3, cfg->GetIndexFromStrArr(config::ConfigType::GROUP, "POS_META"));
    EXPECT_EQ(4, cfg->GetIndexFromStrArr(config::ConfigType::GROUP, "POS_RSC"));
    EXPECT_EQ(-1, cfg->GetIndexFromStrArr(config::ConfigType::GROUP, "INVALIE_NAME"));
    EXPECT_EQ(-1, cfg->GetIndexFromStrArr(config::ConfigType::GROUP, "PERF_PSD"));

    EXPECT_EQ(0, cfg->GetIndexFromStrArr(config::ConfigType::NODE, "PERF_PSD"));
    EXPECT_EQ(1, cfg->GetIndexFromStrArr(config::ConfigType::NODE, "PERF_VOLUME"));
    EXPECT_EQ(2, cfg->GetIndexFromStrArr(config::ConfigType::NODE, "PERF_METAFS"));
    EXPECT_EQ(4, cfg->GetIndexFromStrArr(config::ConfigType::NODE, "PERF_CP"));
    EXPECT_EQ(6, cfg->GetIndexFromStrArr(config::ConfigType::NODE, "LAT_SUBMIT"));
    EXPECT_EQ(8, cfg->GetIndexFromStrArr(config::ConfigType::NODE, "LAT_REBUILD"));
    EXPECT_EQ(13, cfg->GetIndexFromStrArr(config::ConfigType::NODE, "Q_SCHEDULING"));
    EXPECT_EQ(-1, cfg->GetIndexFromStrArr(config::ConfigType::NODE, "INVALIE_NAME"));
    EXPECT_EQ(-1, cfg->GetIndexFromStrArr(config::ConfigType::NODE, "POS_RSC"));
}

TEST_F(ConfigTest, GetIntValueFromEntry)
{
    // DEFAULT
    EXPECT_EQ(1, cfg->GetIntValueFromEntry("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32", "AirBuild"));
    EXPECT_EQ(1, cfg->GetIntValueFromEntry("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32", "NodeBuild"));
    EXPECT_EQ(0, cfg->GetIntValueFromEntry("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:Off, SamplingRatio: 1000, AidSize:32", "NodeRun"));
    EXPECT_EQ(3, cfg->GetIntValueFromEntry("StreamingInterval:  3   , AirBuild  : True, NodeBuild:True, NodeRun:Off, SamplingRatio: 1000, AidSize:32", "StreamingInterval"));
    EXPECT_EQ(1000, cfg->GetIntValueFromEntry("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:off, SamplingRatio: 1000, AidSize:32", "SamplingRatio"));
    EXPECT_EQ(-1, cfg->GetIntValueFromEntry("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:off, SamplingRatio: 1000, AidSize:32", "Condition"));
    EXPECT_EQ(-1, cfg->GetIntValueFromEntry("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:Off, SamplingRatio: 1000, AidSize:32", "NodeName"));

    // GROUP
    EXPECT_EQ(0, cfg->GetIntValueFromEntry("GroupName: POS_RSC, NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr", "NodeBuild"));
    EXPECT_EQ(1, cfg->GetIntValueFromEntry("GroupName: POS_RSC, NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr", "NodeRun"));
    EXPECT_EQ(-1, cfg->GetIntValueFromEntry("GroupName: POS_RSC, NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr", "NodeName"));
    EXPECT_EQ(-1, cfg->GetIntValueFromEntry("GroupName: POS_RSC, NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr", "Condition"));
    EXPECT_EQ(-1, cfg->GetIntValueFromEntry("GroupName: POS_RSC", "GroupName"));

    // NODE
    EXPECT_EQ(1, cfg->GetIntValueFromEntry("NodeName: Q_IOWORER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "NodeBuild"));
    EXPECT_EQ(0, cfg->GetIntValueFromEntry("NodeName: Q_IOWORER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "NodeRun"));
    EXPECT_EQ(-1, cfg->GetIntValueFromEntry("NodeName: Q_IOWORER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "Type"));
    EXPECT_EQ(-1, cfg->GetIntValueFromEntry("NodeName: Q_IOWORER, Type: QUEUE2, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "AirBuild"));
    EXPECT_EQ(10, cfg->GetIntValueFromEntry("NodeName: Q_IOWORER, Type: QUEUE2, NodeBuild: True, NodeRun: Off, Condition: max_depth<100, Delegation: HandleMaxDepth, SamplingRatio: 10", "SamplingRatio"));
}

TEST_F(ConfigTest, GetStrValueFromEntry)
{
    // DEFAULT
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32", "AirBuild").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32", "NodeBuild").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:Off, SamplingRatio: 1000, AidSize:32", "StreamingInterval").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:Off, SamplingRatio: 1000, AidSize:32", "SamplingRatio").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:Off, SamplingRatio: 1000, AidSize:32", "Condition").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:off, SamplingRatio: 1000, AidSize:32", "NodeName").compare(""));

    // // GROUP
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("GroupName: POS_RSC  , NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr", "GroupName").compare("POS_RSC"));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("GroupName: POS_RSC", "GroupName").compare("POS_RSC"));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("GroupName: POS_RSC  , NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr", "NodeRun").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("GroupName: POS_RSC  , NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr", "NodeName").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("GroupName: POS_RSC  , NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr", "Condition").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("GroupName: POS_RSC  , NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr", "NodeRun").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("GroupName: POS_RSC  , NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr", "SamplingRatio").compare(""));

    // NODE
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName:   Q_IOWORKER    , Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "NodeName").compare("Q_IOWORKER"));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName: Q_IOWORKER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "Type").compare("QUEUE"));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName: Q_IOWORKER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth, GroupName:  IO     ", "GroupName").compare("IO"));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName: Q_IOWORKER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "NodeRun").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName: Q_IOWORKER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "SamplingRatio").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName: Q_IOWORKER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "AirBuild").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName: Q_IOWORKER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "NodeBuild").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName: Q_IOWORKER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "GroupName").compare(""));
}

TEST_F(ConfigInterfaceTest, CheckConfigRule)
{
    EXPECT_EQ(0, cfg::CheckConfigRule());
}

TEST_F(ConfigInterfaceTest, GetArrSize)
{
    EXPECT_EQ(1, cfg::GetArrSize(config::ConfigType::DEFAULT));
    EXPECT_EQ(5, cfg::GetArrSize(config::ConfigType::GROUP));
    EXPECT_EQ(15, cfg::GetArrSize(config::ConfigType::NODE));
}

TEST_F(ConfigInterfaceTest, GetName)
{
    EXPECT_EQ(0, cfg::GetName(config::ConfigType::DEFAULT, 0).compare(""));
    EXPECT_EQ(0, cfg::GetName(config::ConfigType::GROUP, 3).compare("POS_META"));
    EXPECT_EQ(0, cfg::GetName(config::ConfigType::NODE, 0).compare("PERF_PSD"));
    EXPECT_EQ(0, cfg::GetName(config::ConfigType::NODE, 1).compare("PERF_VOLUME"));
}

TEST_F(ConfigInterfaceTest, GetIndex)
{
    EXPECT_EQ(0, cfg::GetIndex(config::ConfigType::NODE, "PERF_PSD"));
    EXPECT_EQ(4, cfg::GetIndex(config::ConfigType::NODE, "PERF_CP"));
    EXPECT_EQ(8, cfg::GetIndex(config::ConfigType::NODE, "LAT_REBUILD"));
    EXPECT_EQ(13, cfg::GetIndex(config::ConfigType::NODE, "Q_SCHEDULING"));
    EXPECT_EQ(-1, cfg::GetIndex(config::ConfigType::NODE, "UNKNOWN"));
}

TEST_F(ConfigInterfaceTest, GetIntValue)
{
    EXPECT_EQ(1, cfg::GetIntValue(config::ConfigType::DEFAULT, "StreamingInterval"));
    EXPECT_EQ(1000, cfg::GetIntValue(config::ConfigType::DEFAULT, "SamplingRatio"));
    EXPECT_EQ(32, cfg::GetIntValue(config::ConfigType::DEFAULT, "AidSize"));
    EXPECT_EQ(1, cfg::GetIntValue(config::ConfigType::DEFAULT, "AirBuild"));
    EXPECT_EQ(1, cfg::GetIntValue(config::ConfigType::DEFAULT, "NodeBuild"));
    EXPECT_EQ(1, cfg::GetIntValue(config::ConfigType::DEFAULT, "NodeRun"));

    EXPECT_EQ(0, cfg::GetIntValue(config::ConfigType::GROUP, "NodeBuild", "POS_META"));
    EXPECT_EQ(1, cfg::GetIntValue(config::ConfigType::GROUP, "NodeBuild", "POS"));

    EXPECT_EQ(1, cfg::GetIntValue(config::ConfigType::NODE, "NodeBuild", "PERF_PSD"));
    EXPECT_EQ(1, cfg::GetIntValue(config::ConfigType::NODE, "NodeRun", "PERF_PSD"));
    EXPECT_EQ(0, cfg::GetIntValue(config::ConfigType::NODE, "NodeBuild", "LAT_PSD"));
    EXPECT_EQ(100, cfg::GetIntValue(config::ConfigType::NODE, "SamplingRatio", "LAT_PSD"));
    EXPECT_EQ(1000, cfg::GetIntValue(config::ConfigType::NODE, "SamplingRatio", "LAT_SUBMIT"));
}

TEST_F(ConfigInterfaceTest, GetStrValue)
{
    EXPECT_EQ(0, cfg::GetStrValue(config::ConfigType::NODE, "Type", "PERF_PSD").compare("PERFORMANCE"));
}

TEST_F(ConfigCheckerTest, CheckArrayRule_DefaultType)
{
    constexpr std::string_view default_arr = R"DEFAULT(
    "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"
    )DEFAULT";
    constexpr std::string_view default_arr_with_comment = R"DEFAULT(
            // default setting for Mandatory
    "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"
    )DEFAULT";
    constexpr std::string_view default_arr_multi_line = R"DEFAULT(
            // default setting for Mandatory
    "StreamingInterval:1, AirBuild:True, 
        NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"
    )DEFAULT";
    constexpr std::string_view default_arr_multi_line2 = R"DEFAULT(
            // default setting for Mandatory
    "StreamingInterval:1, AirBuild:True
        ,NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"
    )DEFAULT";

    EXPECT_EQ(0, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_arr));
    EXPECT_EQ(0, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_arr_with_comment));
    EXPECT_EQ(0, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_arr_multi_line));
    EXPECT_EQ(0, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_arr_multi_line2));
}

TEST_F(ConfigCheckerTest, CheckArrayRule_GroupType)
{
    constexpr std::string_view group_arr = R"GROUP(
    "GroupName:POS_IO, Compile:False"
    "GroupName:POS_META"
    "GroupName: testetsetsetset, NodeRun: On, NodeBuild: False"
    "GroupName: POS_REBUILD, SamplingRatio: 1000, NodeBuild: True"
    )GROUP";
    constexpr std::string_view group_arr_with_comment = R"GROUP(
    "GroupName:POS_IO, Compile:False"
    "GroupName:POS_META"// comment test
    "GroupName: testetsetsetset, NodeRun: On, NodeBuild: False"
    "GroupName: POS_REBUILD, SamplingRatio: 1000, NodeBuild: True"
    )GROUP";
    constexpr std::string_view group_arr_multi_line = R"GROUP(
            // group setting, TBD
    "GroupName:POS_IO, Compile:False"
    "GroupName:POS_META"// comment test
    "GroupName: testetsetsetset, NodeRun: On, NodeBuild: False"
    "GroupName: POS_REBUILD
        , SamplingRatio: 1000, NodeBuild: True"
    )GROUP";
    EXPECT_EQ(0, cfg_checker->CheckArrayRule(config::ConfigType::GROUP, group_arr));
    EXPECT_EQ(0, cfg_checker->CheckArrayRule(config::ConfigType::GROUP, group_arr_with_comment));
    EXPECT_EQ(0, cfg_checker->CheckArrayRule(config::ConfigType::GROUP, group_arr_multi_line));
}

TEST_F(ConfigCheckerTest, CheckArrayRule_NodeType)
{
    constexpr std::string_view node_arr = R"NODE(
    "NodeName: Q_SUBMIT, Type:Queue, Compile:True"
    "NodeName:Q_EVENT , Type:Queue, Compile:False"
    "NodeName:PERF_VOLUME, Type:Performance, Compile:True"
    "NodeName: LAT_VOLUME  , Type:Latency, Compile:True"
    "NodeName: Q_REACTOR"
    )NODE";
    constexpr std::string_view node_arr_with_comment = R"NODE(
            // node setting
    "NodeName: Q_SUBMIT, Type:Queue, Compile:True"
    // this comment is okay
    "NodeName:Q_EVENT , Type:Queue, Compile:False"
    // but do not insert comment within double quotation marks!
    "NodeName:PERF_VOLUME, Type:Performance, Compile:True"
    "NodeName: LAT_VOLUME  , Type:Latency, Compile:True"
    "NodeName: Q_REACTOR"   // valid comment
    )NODE";
    constexpr std::string_view node_arr_multi_line = R"NODE(
            // node setting
    "NodeName: Q_SUBMIT, Type:Queue, Compile:True"
    // this comment is okay
    "NodeName:Q_EVENT , Type:Queue, Compile:False"
    // but do not insert comment within double quotation marks!
    "NodeName:PERF_VOLUME,
         
        Type:Performance, 
        Compile:True"
    "NodeName: LAT_VOLUME  , Type:Latency, Compile:True"
    "NodeName: Q_REACTOR"
    )NODE";
    constexpr std::string_view node_arr_multi_line2 = R"NODE(
            // node setting
    "NodeName: Q_SUBMIT, Type:Queue, Compile:True"
    // this comment is okay
    "NodeName:Q_EVENT     , Type:Queue, Compile:False"
    // but do not insert comment within double quotation marks!
    "NodeName:PERF_VOLUME           
         
        ,Type:Performance, 
        Compile:True"
    "NodeName: LAT_VOLUME  , Type:Latency, Compile:True"
    "NodeName: Q_REACTOR"
    )NODE";

    EXPECT_EQ(0, cfg_checker->CheckArrayRule(config::ConfigType::NODE, node_arr));
    EXPECT_EQ(0, cfg_checker->CheckArrayRule(config::ConfigType::NODE, node_arr_with_comment));
    EXPECT_EQ(0, cfg_checker->CheckArrayRule(config::ConfigType::NODE, node_arr_multi_line));
    EXPECT_EQ(0, cfg_checker->CheckArrayRule(config::ConfigType::NODE, node_arr_multi_line2));
}

TEST_F(ConfigCheckerTest, EntryFormatViolation)
{
    constexpr std::string_view default_no_quote = R"DEFAULT(
    StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32
    )DEFAULT";
    constexpr std::string_view default_no_key_viol = R"DEFAULT(
    "StreamingInterval:1, :True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"
    )DEFAULT";
    constexpr std::string_view default_no_value_viol = R"DEFAULT(
    "StreamingInterval:1, AirBuild:, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"
    )DEFAULT";
    constexpr std::string_view default_no_colon_viol = R"DEFAULT(
    "StreamingInterval:1, AirBuild True, NodeBuild True, NodeRun:On, SamplingRatio: 1000, AidSize:32"
    )DEFAULT";
    constexpr std::string_view default_no_colon_viol2 = R"DEFAULT(
    "StreamingInterval 1, AirBuild True, NodeBuild True, NodeRun On, SamplingRatio 1000, AidSize 32"
    )DEFAULT";
    constexpr std::string_view default_comment_viol = R"DEFAULT(
    "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On,   // comment violation
        SamplingRatio: 1000, AidSize:32"
    )DEFAULT";
    constexpr std::string_view default_no_end_quote = R"DEFAULT(
    "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On,
        SamplingRatio: 1000, AidSize:32
    )DEFAULT";
    constexpr std::string_view default_comma_count_viol = R"DEFAULT(
    "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On,
        SamplingRatio: 1000, AidSize:32, "
    )DEFAULT";

    EXPECT_EQ(-1, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_no_quote));
    EXPECT_EQ(-1, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_no_key_viol));
    EXPECT_EQ(-1, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_no_value_viol));
    EXPECT_EQ(-1, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_no_colon_viol));
    EXPECT_EQ(-1, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_no_colon_viol2));
    EXPECT_EQ(-1, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_comment_viol));
    EXPECT_EQ(-1, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_no_end_quote));
    EXPECT_EQ(-1, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_comma_count_viol));
}

TEST_F(ConfigCheckerTest, NameDuplicatedViolation)
{
    constexpr std::string_view group_key_duplicated(R"GROUP(
    "GroupName:POS_IO, Compile:False"
    "GroupName:POS_REBUILD"
    "GroupName: testetsetsetset, NodeRun: on, NodeBuild: False"
    "GroupName: POS_REBUILD, SamplingRatio: 1000, NodeBuild: True"
    )GROUP");
    EXPECT_EQ(-2, cfg_checker->CheckArrayRule(config::ConfigType::GROUP, group_key_duplicated));

    constexpr std::string_view node_key_duplicated = R"NODE(
    "NodeName: Q_SUBMIT, Type:Queue, Compile:True"
    "NodeName:Q_EVENT , Type:Queue, Compile:False"
    "NodeName:PERF_VOLUME, Type:Performance, Compile:True"
    "NodeName: LAT_VOLUME  , Type:Latency, Compile:True"
    "NodeName: Q_EVENT"
    )NODE";
    EXPECT_EQ(-2, cfg_checker->CheckArrayRule(config::ConfigType::NODE, node_key_duplicated));
}

TEST_F(ConfigCheckerTest, CheckKeyRule_DefaultType)
{
    // DEFAULT
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "SamplingRatio: 1000, NodeRun:On, NodeBuild:True, AirBuild:True, StreamingInterval:1, AidSize:32"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "SamplingRatio: 1000, NodeRun:On, NodeBuild:True, AirBuild:True, StreamingInterval:1, AidSize : 32"));
}

TEST_F(ConfigCheckerTest, CheckKeyRule_GroupType)
{
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, Delegation:HandleRsc"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "NodeBuild: False, GroupName: POS_RSC, Delegation:HandleRsc, SamplingRatio: 10, NodeRun: On, Condition: nullptr != logged_data"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "NodeBuild: False, NodeRun: On, SamplingRatio: 10, GroupName: POS_RSC, Condition: nullptr!=logged_data"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 100"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, NodeRun: On"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC"));
}

TEST_F(ConfigCheckerTest, CheckKeyRule_NodeType)
{
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "Type: PERFORMANCE, GroupName: POS_Journal, NodeRun: On, Condition: 100MB/s<write_bw, NodeBuild: True, NodeName: PERF_CP, Delegation: HandleLowBW"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: UNGROUPED"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, GroupName: UNGROUPED"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, GroupName: UNGROUPED"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, GroupName: UNGROUPED"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, GroupName: UNGROUPED"));
}

TEST_F(ConfigCheckerTest, KeyTypoViolation_DefaultType)
{
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AAABuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, N0deBuild:True, NodeRun:On, SamplingRati0: 1000, AidSize:32"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NR:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "SI:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SR: 0.001, AidSize:32"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AS:33"));
}

TEST_F(ConfigCheckerTest, KeyTypoViolation_GroupType)
{
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NB: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, Delegation : HandleRsc"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, RN: On, SamplingRatio: 10, Condition: nullptr!=logged_data, Delegation : HandleRsc"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SR: 10, Condition: nullptr!=logged_data, Delegation : HandleRsc"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, cond: nullptr!=logged_data, Delegation : HandleRsc"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, DelegateFunc : HandleRsc"));
}

TEST_F(ConfigCheckerTest, KeyTypoViolation_NodeType)
{
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NN: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal, SamplingRatio: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP, t: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal, SamplingRatio: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, NB: True, NodeRun: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal, SamplingRatio: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, RN: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal, SamplingRatio: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, cond: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal, SamplingRatio: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, del: HandleLowBW, GroupName: POS_Journal, SamplingRatio: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GN: POS_Journal, SamplingRatio: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal, SR: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, SamplingRatio: 10"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: LAT_TEST, Type: LATENCY, SamplingRatio : 1000"));
}

TEST_F(ConfigCheckerTest, KeyMandatoryViolation_DefaultType)
{
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, AidSize:32"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, SamplingRatio:1000, AidSize:32"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeRun:On, SamplingRatio:1000, AidSize: 50"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "StreamingInterval:1, NodeBuild:True, NodeRun:On, SamplingRatio:1000, AidSize:32"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio:1000, AidSize:10"));
}

TEST_F(ConfigCheckerTest, KeyMandatoryViolation_GroupType)
{
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "NodeBuild: False, NodeRun: On, SamplingRatio: 10"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "NodeBuild: False, SamplingRatio: 10"));
}

TEST_F(ConfigCheckerTest, KeyMandatoryViolation_NodeType)
{
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "Type: PERFORMANCE"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, Type: PERFORMANCE"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "Condition: 100MB/s<write_bw, NodeRun: On, Delegation: HandleLowBW, NodeName: PERF_CP"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "Condition: 100MB/s<write_bw, NodeRun: On, Delegation: HandleLowBW"));
}

TEST_F(ConfigCheckerTest, KeyDuplicationViolation_DefaultType)
{
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "StreamingInterval:1, StreamingInterval: 10, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, AirBuild: False, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, NodeBuild:True, AidSize:32"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, NodeRun:On, AidSize:32"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "SamplingRatio: 10, StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, "SamplingRatio: 10, AidSize: 100, StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, AidSize: 10"));
}

TEST_F(ConfigCheckerTest, KeyDuplicationViolation_GroupType)
{
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, GroupName: POS_RSC, Delegation : HandleRsc"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, NodeBuild: False, Delegation : HandleRsc"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, NodeRun: On, Delegation : HandleRsc"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, SamplingRatio: 1, Delegation : HandleRsc"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, Condition:nullptr<logged_data, Delegation : HandleRsc"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, Delegation : HandleRsc, Delegation : HandleRsc"));
}

TEST_F(ConfigCheckerTest, KeyDuplicationViolation_NodeType)
{
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: LAT_CP, Type: LATENCY, NodeBuild: True, NodeRun: On, Condition: logged_data<lat_mean, Delegation: HandleLowBW, GroupName: POS_Journal, NodeName: LAT_CP"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: LAT_CP, Type: LATENCY, Type: LATENCY, NodeBuild: True, NodeRun: On, Condition: logged_data<lat_mean, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: LAT_CP, Type: LATENCY, NodeBuild: True, NodeBuild: True, NodeRun: On, Condition: logged_data<lat_mean, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: LAT_CP, Type: LATENCY, NodeBuild: True, NodeRun: On, NodeRun: On, Condition: logged_data<lat_mean, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: LAT_CP, Type: LATENCY, NodeBuild: True, NodeRun: On, Condition: logged_data<lat_mean, Condition: logged_data<lat_mean, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: LAT_CP, Type: LATENCY, NodeBuild: True, NodeRun: On, Condition: logged_data<lat_mean, Delegation: HandleLowBW, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: LAT_CP, Type: LATENCY, NodeBuild: True, NodeRun: On, Condition: logged_data<lat_mean, Delegation: HandleLowBW, GroupName: POS_Journal, GroupName: POS_Journal"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "SamplingRatio:1000, NodeName: QUEUE_CP, Type: QUEUE, NodeBuild: True, NodeRun: On, Condition: logged_data<depth, Delegation: HandleLowQD, GroupName: POS_Journal, SamplingRatio:1000"));
}

TEST_F(ConfigCheckerTest, CheckGroupNameInNodePass)
{
    constexpr std::string_view group_str = R"DEFAULT(
    // group setting for Mandatory & Optional
        "GroupName: POS"
        "GroupName: POS_JOURNAL
, NodeBuild: True"
        "GroupName: POS_IO, NodeBuild: True, NodeRun: Off"
        "GroupName: POS_META, NodeBuild: False, NodeRun: On, SamplingRatio: 100"
        "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr"
    )DEFAULT";

    constexpr std::string_view entry1 = R"DEFAULT(
    NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: write_bw>100MB/s, Delegation: HandleLowBW, GroupName: POS_JOURNAL
    )DEFAULT";

    constexpr std::string_view entry2 = R"DEFAULT(
        NodeName: LAT_REBUILD, Type: LATENCY, NodeBuild: True, NodeRun: On, 
            GroupName: POS_RSC
    )DEFAULT";

    constexpr std::string_view entry3 = R"DEFAULT(
        NodeName: Q_COMPLETION,        Type: QUEUE,            GroupName: POS_IO,    NodeBuild: False
    )DEFAULT";

    EXPECT_EQ(0, cfg_checker->CheckGroupNameInNode(entry1, group_str));
    EXPECT_EQ(0, cfg_checker->CheckGroupNameInNode(entry2, group_str));
    EXPECT_EQ(0, cfg_checker->CheckGroupNameInNode(entry3, group_str));
    EXPECT_EQ(-3, cfg_checker->CheckGroupNameInNode("NodeName: PERF_TEST", group_str));
    EXPECT_EQ(-3, cfg_checker->CheckGroupNameInNode("NodeName: PERF_TEST, GroupName: ERR", group_str));
}

TEST_F(ConfigCheckerTest, CheckValueRulePass)
{
    EXPECT_EQ(0, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize: 30"));
    EXPECT_EQ(0, cfg_checker->CheckValueRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 100, Condition: nullptr!=logged_data, Delegation: HandleRsc"));
    EXPECT_EQ(0, cfg_checker->CheckValueRule(config::ConfigType::GROUP, "GroupName: POS_RSC"));
    EXPECT_EQ(0, cfg_checker->CheckValueRule(config::ConfigType::NODE, "NodeName: LAT_CP, Type: LATENCY, NodeBuild: True, NodeRun: On, SamplingRatio:1000, Condition: 100MB/s<lat_mean, Delegation: HandleLat, GroupName: POS_Journal"));
}

TEST_F(ConfigCheckerTest, ValueVaildityViolation_DefaultType)
{
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:99, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval: a, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval: 1a, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval: aaaaaa, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild: 100, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:-99, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:abcd, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:abcd, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:abcd, SamplingRatio: 10000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: -1, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 0.0000000000000001, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 0.a001, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1ra01, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize: -10"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 100000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize: 10000000000"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize: 5000000000"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize: 4294967296"));
}

TEST_F(ConfigCheckerTest, ValueVaildityViolation_GroupType)
{
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::GROUP, "GroupName: POS_RSCCCCCCCCCCCCC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr != logged_data, Delegation: HandleRsc"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: 123, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, Delegation: HandleRsc"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, NodeRun: 123, SamplingRatio: 10, Condition: nullptr!=logged_data, Delegation: HandleRsc"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::GROUP, "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 0, Condition: nullptr!=logged_data, Delegation: HandleRsc"));
}

TEST_F(ConfigCheckerTest, ValueVaildityViolation_NodeType)
{
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::NODE, "NodeName: PERF_CPPPPPPPPPPPPP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, SamplingRatio:1000, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: NOTYPE, NodeBuild: True, NodeRun: On, SamplingRatio:1000, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: 123, NodeRun: On, SamplingRatio:1000, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: 123, SamplingRatio:1000, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, SamplingRatio:0.9, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, SamplingRatio:1000, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journallllllllll"));
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
