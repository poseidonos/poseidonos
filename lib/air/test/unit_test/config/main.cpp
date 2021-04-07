
#include <stdio.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "config_interface_test.h"
#include "config_checker_test.h"
#include "config_test.h"

TEST_F(ConfigCheckerTest, CheckArrayRulePass)
{
    constexpr config::String default_arr = R"DEFAULT(
    "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"
    )DEFAULT";
    constexpr config::String default_arr_with_comment = R"DEFAULT(
            // default setting for Mandatory
    "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"
    )DEFAULT";
    constexpr config::String default_arr_multi_line = R"DEFAULT(
            // default setting for Mandatory
    "StreamingInterval:1, AirBuild:True, 
        NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"
    )DEFAULT";
    constexpr config::String default_arr_multi_line2 = R"DEFAULT(
            // default setting for Mandatory
    "StreamingInterval:1, AirBuild:True
        ,NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"
    )DEFAULT";

    EXPECT_EQ(0, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_arr));
    EXPECT_EQ(0, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_arr_with_comment));
    EXPECT_EQ(0, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_arr_multi_line));
    EXPECT_EQ(0, cfg_checker->CheckArrayRule(config::ConfigType::DEFAULT, default_arr_multi_line2));
    
    constexpr config::String group_arr = R"GROUP(
    "GroupName:POS_IO, Compile:False"
    "GroupName:POS_META"
    "GroupName: testetsetsetset, NodeRun: On, NodeBuild: False"
    "GroupName: POS_REBUILD, SamplingRatio: 1000, NodeBuild: True"
    )GROUP";
    constexpr config::String group_arr_with_comment = R"GROUP(
    "GroupName:POS_IO, Compile:False"
    "GroupName:POS_META"// comment test
    "GroupName: testetsetsetset, NodeRun: On, NodeBuild: False"
    "GroupName: POS_REBUILD, SamplingRatio: 1000, NodeBuild: True"
    )GROUP";
    constexpr config::String group_arr_multi_line = R"GROUP(
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

    constexpr config::String node_arr = R"NODE(
    "NodeName: Q_SUBMIT, Type:Queue, Compile:True"
    "NodeName:Q_EVENT , Type:Queue, Compile:False"
    "NodeName:PERF_VOLUME, Type:Performance, Compile:True"
    "NodeName: LAT_VOLUME  , Type:Latency, Compile:True"
    "NodeName: Q_REACTOR"
    )NODE";
    constexpr config::String node_arr_with_comment = R"NODE(
            // node setting
    "NodeName: Q_SUBMIT, Type:Queue, Compile:True"
    // this comment is okay
    "NodeName:Q_EVENT , Type:Queue, Compile:False"
    // but do not insert comment within double quotation marks!
    "NodeName:PERF_VOLUME, Type:Performance, Compile:True"
    "NodeName: LAT_VOLUME  , Type:Latency, Compile:True"
    "NodeName: Q_REACTOR"   // valid comment
    )NODE";
    constexpr config::String node_arr_multi_line = R"NODE(
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
    constexpr config::String node_arr_multi_line2 = R"NODE(
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
    constexpr config::String default_no_quote = R"DEFAULT(
    StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32
    )DEFAULT";
    constexpr config::String default_no_key_viol = R"DEFAULT(
    "StreamingInterval:1, :True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"
    )DEFAULT";
    constexpr config::String default_no_value_viol = R"DEFAULT(
    "StreamingInterval:1, AirBuild:, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"
    )DEFAULT";
    constexpr config::String default_no_colon_viol = R"DEFAULT(
    "StreamingInterval:1, AirBuild True, NodeBuild True, NodeRun:On, SamplingRatio: 1000, AidSize:32"
    )DEFAULT";
    constexpr config::String default_no_colon_viol2 = R"DEFAULT(
    "StreamingInterval 1, AirBuild True, NodeBuild True, NodeRun On, SamplingRatio 1000, AidSize 32"
    )DEFAULT";
    constexpr config::String default_comment_viol = R"DEFAULT(
    "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On,   // comment violation
        SamplingRatio: 1000, AidSize:32"
    )DEFAULT";
    constexpr config::String default_no_end_quote = R"DEFAULT(
    "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On,
        SamplingRatio: 1000, AidSize:32
    )DEFAULT";
    constexpr config::String default_comma_count_viol = R"DEFAULT(
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
    config::String group_key_duplicated(R"GROUP(
    "GroupName:POS_IO, Compile:False"
    "GroupName:POS_REBUILD"
    "GroupName: testetsetsetset, NodeRun: on, NodeBuild: False"
    "GroupName: POS_REBUILD, SamplingRatio: 1000, NodeBuild: True"
    )GROUP");
    EXPECT_EQ(-2, cfg_checker->CheckArrayRule(config::ConfigType::GROUP, group_key_duplicated));

    constexpr config::String node_key_duplicated = R"NODE(
    "NodeName: Q_SUBMIT, Type:Queue, Compile:True"
    "NodeName:Q_EVENT , Type:Queue, Compile:False"
    "NodeName:PERF_VOLUME, Type:Performance, Compile:True"
    "NodeName: LAT_VOLUME  , Type:Latency, Compile:True"
    "NodeName: Q_EVENT"
    )NODE";
    EXPECT_EQ(-2, cfg_checker->CheckArrayRule(config::ConfigType::NODE, node_key_duplicated));
}

TEST_F(ConfigCheckerTest, CheckKeyRulePass)
{
    // DEFAULT
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, 
                        "SamplingRatio: 1000, NodeRun:On, NodeBuild:True, AirBuild:True, StreamingInterval:1, AidSize:32"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, 
                        "SamplingRatio: 1000, NodeRun:On, NodeBuild:True, AirBuild:True, StreamingInterval:1, AidSize : 32"));    
    // GROUP
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, 
                    "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, Delegation:HandleRsc"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, 
                    "NodeBuild: False, GroupName: POS_RSC, Delegation:HandleRsc, SamplingRatio: 10, NodeRun: On, Condition: nullptr != logged_data"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, 
                    "NodeBuild: False, NodeRun: On, SamplingRatio: 10, GroupName: POS_RSC, Condition: nullptr!=logged_data"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, 
                    "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 100"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::GROUP,
                    "GroupName: POS_RSC, NodeBuild: False, NodeRun: On"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::GROUP,
                    "GroupName: POS_RSC, NodeBuild: False"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::GROUP,
                    "GroupName: POS_RSC"));

    // NODE
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "Type: PERFORMANCE, GroupName: POS_Journal, NodeRun: On, Condition: 100MB/s<write_bw, NodeBuild: True, NodeName: PERF_CP, Delegation: HandleLowBW"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: UNGROUPED"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, GroupName: UNGROUPED"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, GroupName: UNGROUPED"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, GroupName: UNGROUPED"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP, Type: PERFORMANCE, GroupName: UNGROUPED"));
}

TEST_F(ConfigCheckerTest, KeyTypoViolation)
{
    // DEFAULT
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AAABuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, N0deBuild:True, NodeRun:On, SamplingRati0: 1000, AidSize:32"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NR:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT,
                        "SI:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SR: 0.001, AidSize:32"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AS:33"));

    // GROUP
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, 
                        "GroupName: POS_RSC, NB: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, Delegation : HandleRsc"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, 
                        "GroupName: POS_RSC, NodeBuild: False, RN: On, SamplingRatio: 10, Condition: nullptr!=logged_data, Delegation : HandleRsc"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, 
                        "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SR: 10, Condition: nullptr!=logged_data, Delegation : HandleRsc"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, 
                        "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, cond: nullptr!=logged_data, Delegation : HandleRsc"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, 
                        "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, DelegateFunc : HandleRsc"));
    
    // Node
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NN: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal, SamplingRatio: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, t: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal, SamplingRatio: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, NB: True, NodeRun: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal, SamplingRatio: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, RN: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal, SamplingRatio: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, cond: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal, SamplingRatio: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, del: HandleLowBW, GroupName: POS_Journal, SamplingRatio: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GN: POS_Journal, SamplingRatio: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal, SR: 100"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, SamplingRatio: 10"));
    EXPECT_EQ(-3, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: LAT_TEST, Type: LATENCY, SamplingRatio : 1000"));
}

TEST_F(ConfigCheckerTest, KeyMandatoryViolation)
{
    // DEFAULT
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, 
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, AidSize:32"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, SamplingRatio:1000, AidSize:32"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, 
                        "StreamingInterval:1, AirBuild:True, NodeRun:On, SamplingRatio:1000, AidSize: 50"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, 
                        "StreamingInterval:1, NodeBuild:True, NodeRun:On, SamplingRatio:1000, AidSize:32"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT, 
                        "AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio:1000, AidSize:10"));

    // GROUP
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::GROUP,
                        "NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::GROUP,
                        "NodeBuild: False, NodeRun: On, SamplingRatio: 10"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::GROUP,
                        "NodeBuild: False, SamplingRatio: 10"));

    // NODE
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "NodeName: PERF_CP"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::NODE, "Type: PERFORMANCE"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeBuild: True, NodeRun: On, Condition: 100MB/s<write_bw, Type: PERFORMANCE"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "Condition: 100MB/s<write_bw, NodeRun: On, Delegation: HandleLowBW, NodeName: PERF_CP"));
    EXPECT_EQ(-4, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "Condition: 100MB/s<write_bw, NodeRun: On, Delegation: HandleLowBW"));
}

TEST_F(ConfigCheckerTest, KeyDuplicationViolation)
{
    // DEFAULT
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, StreamingInterval: 10, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, AirBuild: False, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, NodeBuild:True, AidSize:32"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, NodeRun:On, AidSize:32"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT,
                        "SamplingRatio: 10, StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::DEFAULT,
                        "SamplingRatio: 10, AidSize: 100, StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, AidSize: 10"));

    // GROUP
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, 
                        "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, GroupName: POS_RSC, Delegation : HandleRsc"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, 
                        "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, NodeBuild: False, Delegation : HandleRsc"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, 
                        "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, NodeRun: On, Delegation : HandleRsc"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, 
                        "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, SamplingRatio: 1, Delegation : HandleRsc"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, 
                        "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, Condition:nullptr<logged_data, Delegation : HandleRsc"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::GROUP, 
                        "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, Delegation : HandleRsc, Delegation : HandleRsc"));
    
    // NODE
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: LAT_CP, Type: LATENCY, NodeBuild: True, NodeRun: On, Condition: logged_data<lat_mean, Delegation: HandleLowBW, GroupName: POS_Journal, NodeName: LAT_CP"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: LAT_CP, Type: LATENCY, Type: LATENCY, NodeBuild: True, NodeRun: On, Condition: logged_data<lat_mean, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: LAT_CP, Type: LATENCY, NodeBuild: True, NodeBuild: True, NodeRun: On, Condition: logged_data<lat_mean, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: LAT_CP, Type: LATENCY, NodeBuild: True, NodeRun: On, NodeRun: On, Condition: logged_data<lat_mean, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: LAT_CP, Type: LATENCY, NodeBuild: True, NodeRun: On, Condition: logged_data<lat_mean, Condition: logged_data<lat_mean, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: LAT_CP, Type: LATENCY, NodeBuild: True, NodeRun: On, Condition: logged_data<lat_mean, Delegation: HandleLowBW, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "NodeName: LAT_CP, Type: LATENCY, NodeBuild: True, NodeRun: On, Condition: logged_data<lat_mean, Delegation: HandleLowBW, GroupName: POS_Journal, GroupName: POS_Journal"));
    EXPECT_EQ(-5, cfg_checker->CheckKeyRule(config::ConfigType::NODE,
                        "SamplingRatio:1000, NodeName: QUEUE_CP, Type: QUEUE, NodeBuild: True, NodeRun: On, Condition: logged_data<depth, Delegation: HandleLowQD, GroupName: POS_Journal, SamplingRatio:1000"));
}

TEST_F(ConfigCheckerTest, CheckGroupNameInNodePass)
{
    constexpr config::String group_str = R"DEFAULT(
    // group setting for Mandatory & Optional
        "GroupName: POS"
        "GroupName: POS_JOURNAL
, NodeBuild: True"
        "GroupName: POS_IO, NodeBuild: True, NodeRun: Off"
        "GroupName: POS_META, NodeBuild: False, NodeRun: On, SamplingRatio: 100"
        "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr"
    )DEFAULT";

    constexpr config::String entry1 = R"DEFAULT(
    NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: write_bw>100MB/s, Delegation: HandleLowBW, GroupName: POS_JOURNAL
    )DEFAULT";

    constexpr config::String entry2 = R"DEFAULT(
        NodeName: LAT_REBUILD, Type: LATENCY, NodeBuild: True, NodeRun: On, 
            GroupName: POS_RSC
    )DEFAULT";

    EXPECT_EQ(0, cfg_checker->CheckGroupNameInNode(entry1, group_str));
    EXPECT_EQ(0, cfg_checker->CheckGroupNameInNode(entry2, group_str));
    EXPECT_EQ(0, cfg_checker->CheckGroupNameInNode("NodeName: PERF_TEST", group_str));
    EXPECT_EQ(-3, cfg_checker->CheckGroupNameInNode("NodeName: PERF_TEST, GroupName: ERR", group_str));
}

TEST_F(ConfigCheckerTest, CheckValueRulePass)
{
    EXPECT_EQ(0, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                    "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize: 30"));
    EXPECT_EQ(0, cfg_checker->CheckValueRule(config::ConfigType::GROUP, 
                    "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 100, Condition: nullptr!=logged_data, Delegation: HandleRsc"));
    EXPECT_EQ(0, cfg_checker->CheckValueRule(config::ConfigType::GROUP, "GroupName: POS_RSC"));
    EXPECT_EQ(0, cfg_checker->CheckValueRule(config::ConfigType::NODE,
                    "NodeName: LAT_CP, Type: LATENCY, NodeBuild: True, NodeRun: On, SamplingRatio:1000, Condition: 100MB/s<lat_mean, Delegation: HandleLat, GroupName: POS_Journal"));
}

TEST_F(ConfigCheckerTest, ValueVaildityViolation)
{
    // Default
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:99, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval: a, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval: 1a, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval: aaaaaa, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild: 100, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:-99, NodeRun:On, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:abcd, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:abcd, SamplingRatio: 1000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:abcd, SamplingRatio: 10000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: -1, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 0.0000000000000001, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 0.a001, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1ra01, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize: -10"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 100000, AidSize:32"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize: 10000000000"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize: 5000000000"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::DEFAULT,
                        "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize: 4294967296"));

    // Group
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::GROUP, 
                        "GroupName: POS_RSCCCCCCCCCCCCC, NodeBuild: False, NodeRun: On, SamplingRatio: 10, Condition: nullptr != logged_data, Delegation: HandleRsc"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::GROUP, 
                        "GroupName: POS_RSC, NodeBuild: 123, NodeRun: On, SamplingRatio: 10, Condition: nullptr!=logged_data, Delegation: HandleRsc"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::GROUP, 
                        "GroupName: POS_RSC, NodeBuild: False, NodeRun: 123, SamplingRatio: 10, Condition: nullptr!=logged_data, Delegation: HandleRsc"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::GROUP, 
                        "GroupName: POS_RSC, NodeBuild: False, NodeRun: On, SamplingRatio: 0, Condition: nullptr!=logged_data, Delegation: HandleRsc"));
    
    // Node 
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::NODE,
                        "NodeName: PERF_CPPPPPPPPPPPPP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, SamplingRatio:1000, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: NOTYPE, NodeBuild: True, NodeRun: On, SamplingRatio:1000, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: 123, NodeRun: On, SamplingRatio:1000, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: 123, SamplingRatio:1000, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, SamplingRatio:0.9, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journal"));
    EXPECT_EQ(-6, cfg_checker->CheckValueRule(config::ConfigType::NODE,
                        "NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, SamplingRatio:1000, Condition: 100MB/s<write_bw, Delegation: HandleLowBW, GroupName: POS_Journallllllllll"));
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

    EXPECT_EQ(0, cfg->GetIndexFromStrArr(config::ConfigType::WEB, "PERFORMANCE"));
    EXPECT_EQ(1, cfg->GetIndexFromStrArr(config::ConfigType::WEB, "LATENCY"));
    EXPECT_EQ(2, cfg->GetIndexFromStrArr(config::ConfigType::WEB, "QUEUE"));
    EXPECT_EQ(-1, cfg->GetIndexFromStrArr(config::ConfigType::WEB, "SSD"));
}

TEST_F(ConfigTest, GetEntryFromStrArr)
{
    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::DEFAULT).Compare("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32"));

    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::GROUP, 1).Compare("GroupName: POS_JOURNAL\n, NodeBuild: True"));
    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::GROUP, 3).Compare("GroupName: POS_META, NodeBuild: False, NodeRun: On, SamplingRatio: 100"));
    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::GROUP, 5).Compare(""));

    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::NODE, 1).Compare("NodeName: PERF_VOLUME, Type: PERFORMANCE, NodeBuild: True"));
    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::NODE, 4).Compare("NodeName: PERF_CP, Type: PERFORMANCE, NodeBuild: True, NodeRun: On, Condition: write_bw>100MB/s, Delegation: HandleLowBW, GroupName: POS_JOURNAL"));
    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::NODE, 9).Compare("NodeName: LAT_CP, Type: LATENCY, NodeBuild: False, NodeRun: Off, Condition: 4nine>1sec, Delegation: HandleTailLatency"));
    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::NODE, 10).Compare("NodeName: Q_SUBMIT, Type: QUEUE"));
    EXPECT_EQ(0, cfg->GetEntryFromStrArr(config::ConfigType::GROUP, 99).Compare(""));
}

TEST_F(ConfigTest, GetIntValueFromEntry)
{
    // DEFAULT
    EXPECT_EQ(1, cfg->GetIntValueFromEntry("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32",
                                                "AirBuild"));
    EXPECT_EQ(1, cfg->GetIntValueFromEntry("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32",
                                                "NodeBuild"));
    EXPECT_EQ(0, cfg->GetIntValueFromEntry("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:Off, SamplingRatio: 1000, AidSize:32",
                                                "NodeRun"));
    EXPECT_EQ(3, cfg->GetIntValueFromEntry("StreamingInterval:3, AirBuild  : True, NodeBuild:True, NodeRun:Off, SamplingRatio: 1000, AidSize:32",
                                                "StreamingInterval"));
    EXPECT_EQ(1000, cfg->GetIntValueFromEntry("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:off, SamplingRatio: 1000, AidSize:32",
                                                "SamplingRatio"));
    EXPECT_EQ(-1, cfg->GetIntValueFromEntry("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:off, SamplingRatio: 1000, AidSize:32",
                                                "Condition"));
    EXPECT_EQ(-1, cfg->GetIntValueFromEntry("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:Off, SamplingRatio: 1000, AidSize:32",
                                                "NodeName"));

    // GROUP
    EXPECT_EQ(0, cfg->GetIntValueFromEntry("GroupName: POS_RSC, NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr",
                                            "NodeBuild"));
    EXPECT_EQ(1, cfg->GetIntValueFromEntry("GroupName: POS_RSC, NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr",
                                            "NodeRun"));
    EXPECT_EQ(-1, cfg->GetIntValueFromEntry("GroupName: POS_RSC, NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr",
                                            "NodeName"));
    EXPECT_EQ(-1, cfg->GetIntValueFromEntry("GroupName: POS_RSC, NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr",
                                            "Condition"));
    EXPECT_EQ(-1, cfg->GetIntValueFromEntry("GroupName: POS_RSC", "GroupName"));

    // NODE
    EXPECT_EQ(1, cfg->GetIntValueFromEntry("NodeName: Q_IOWORER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth",
                                            "NodeBuild"));
    EXPECT_EQ(0, cfg->GetIntValueFromEntry("NodeName: Q_IOWORER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth",
                                            "NodeRun"));
    EXPECT_EQ(-1, cfg->GetIntValueFromEntry("NodeName: Q_IOWORER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth",
                                            "Type"));
    EXPECT_EQ(-1, cfg->GetIntValueFromEntry("NodeName: Q_IOWORER, Type: QUEUE2, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth",
                                            "AirBuild"));
    EXPECT_EQ(10, cfg->GetIntValueFromEntry("NodeName: Q_IOWORER, Type: QUEUE2, NodeBuild: True, NodeRun: Off, Condition: max_depth<100, Delegation: HandleMaxDepth, SamplingRatio: 10",
                                            "SamplingRatio"));
}

TEST_F(ConfigTest, GetStrValueFromEntry)
{
    // DEFAULT
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32",
                                                "AirBuild").Compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, SamplingRatio: 1000, AidSize:32",
                                                "NodeBuild").Compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:Off, SamplingRatio: 1000, AidSize:32",
                                                "StreamingInterval").Compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:Off, SamplingRatio: 1000, AidSize:32",
                                                "SamplingRatio").Compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:Off, SamplingRatio: 1000, AidSize:32",
                                                "Condition").Compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:off, SamplingRatio: 1000, AidSize:32",
                                                "NodeName").Compare(""));

    // // GROUP
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("GroupName: POS_RSC  , NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr",
                                            "GroupName").Compare("POS_RSC"));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("GroupName: POS_RSC", "GroupName").Compare("POS_RSC"));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("GroupName: POS_RSC  , NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr",
                                            "NodeRun").Compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("GroupName: POS_RSC  , NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr",
                                            "NodeName").Compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("GroupName: POS_RSC  , NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr",
                                            "Condition").Compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("GroupName: POS_RSC  , NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr",
                                            "NodeRun").Compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("GroupName: POS_RSC  , NodeBuild: False, NodeRun: On, Condition: logged_data!=nullptr",
                                        "SamplingRatio").Compare(""));

    // NODE
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName: Q_IOWORKER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "NodeName").Compare("Q_IOWORKER"));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName: Q_IOWORKER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "Type").Compare("QUEUE"));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName: Q_IOWORKER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth, GroupName:IO", "GroupName").Compare("IO"));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName: Q_IOWORKER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "NodeRun").Compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName: Q_IOWORKER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "SamplingRatio").Compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName: Q_IOWORKER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "AirBuild").Compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName: Q_IOWORKER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "NodeBuild").Compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromEntry("NodeName: Q_IOWORKER, Type: QUEUE, NodeBuild: True, NodeRun: Off, SamplingRatio: 10, Condition: max_depth<100, Delegation: HandleMaxDepth", "GroupName").Compare(""));
}

/*
TEST_F(ConfigInterfaceTest, NodeTest)
{
    std::cout << cfg::CheckConfigRule() << std::endl;
    std::cout << "default count : " << cfg::GetArrSize(config::ConfigType::DEFAULT) << std::endl;
    std::cout << "group count : " << cfg::GetArrSize(config::ConfigType::GROUP) << std::endl;
    std::cout << "node count : " << cfg::GetArrSize(config::ConfigType::NODE) << std::endl;

    for (uint32_t i = 0; i < 7; i++)
        std::cout << cfg::GetName(config::ConfigType::NODE, i) << "." << std::endl;
    std::cout << cfg::GetIndex(config::ConfigType::NODE, "PERF_BENCHMARK") << std::endl;
    std::cout << cfg::GetIndex(config::ConfigType::NODE, "LAT_SUBMIT") << std::endl;
    std::cout << cfg::GetIndex(config::ConfigType::NODE, "LAT_PROCESS") << std::endl;
    std::cout << cfg::GetIndex(config::ConfigType::NODE, "LAT_COMPLETE") << std::endl;
    std::cout << cfg::GetIndex(config::ConfigType::NODE, "LAT_IO_PATH") << std::endl;
    std::cout << cfg::GetIndex(config::ConfigType::NODE, "Q_SUBMISSION") << std::endl;
    std::cout << cfg::GetIndex(config::ConfigType::NODE, "Q_COMPLETION") << std::endl;

    std::cout << cfg::GetIntValue(config::ConfigType::DEFAULT, "StreamingInterval") << std::endl;
    std::cout << cfg::GetIntValue(config::ConfigType::DEFAULT, "SamplingRatio") << std::endl;
    std::cout << cfg::GetIntValue(config::ConfigType::DEFAULT, "AidSize") << std::endl;
    std::cout << cfg::GetIntValue(config::ConfigType::DEFAULT, "AirBuild") << std::endl;
    std::cout << cfg::GetIntValue(config::ConfigType::DEFAULT, "NodeBuild") << std::endl;
    std::cout << cfg::GetIntValue(config::ConfigType::DEFAULT, "NodeRun") << std::endl;

    std::cout << cfg::GetStrValue(config::ConfigType::NODE, "Type", "PERF_BENCHMARK") << ","
    << cfg::GetIntValue(config::ConfigType::NODE, "NodeBuild", "PERF_BENCHMARK") << ","
    << cfg::GetIntValue(config::ConfigType::NODE, "NodeRun", "PERF_BENCHMARK")
    << std::endl;

    std::cout << cfg::GetStrValue(config::ConfigType::NODE, "Type", "LAT_IO_PATH") << ","
    << cfg::GetIntValue(config::ConfigType::NODE, "NodeBuild", "LAT_IO_PATH") << ","
    << cfg::GetIntValue(config::ConfigType::NODE, "NodeRun", "LAT_IO_PATH")
    << std::endl;


    std::cout << cfg::GetStrValue(config::ConfigType::NODE, "Type", "Q_COMPLETION") << ","
    << cfg::GetIntValue(config::ConfigType::NODE, "NodeBuild", "Q_COMPLETION") << ","
    << cfg::GetIntValue(config::ConfigType::NODE, "NodeRun", "Q_COMPLETION")
    << std::endl;

    for (uint32_t i = 0; i < 7; i++)
        std::cout << cfg::GetIntValue(config::ConfigType::NODE, "NodeBuild", cfg::GetName(config::ConfigType::NODE, i)) << " ";
    std::cout << "\n";

    for (uint32_t i = 0; i < 7; i++)
        std::cout << cfg::GetIntValue(config::ConfigType::NODE, "NodeBuild", i) << " ";
    std::cout << "\n";
}
*/

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
