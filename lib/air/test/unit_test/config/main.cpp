
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include <iostream>

#include "config_checker_test.h"
#include "config_interface_test.h"
#include "config_parser_test.h"

TEST_F(ConfigParserTest, GetSentenceFromParagraph)
{
    EXPECT_EQ(0, cfg->GetSentenceFromParagraph(config::ParagraphType::DEFAULT).compare("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On,\n         NodeSamplingRatio: 1000, NodeIndexSize:32"));

    EXPECT_EQ(0, cfg->GetSentenceFromParagraph(config::ParagraphType::GROUP, 1).compare("Group: POS_JOURNAL ,\nNodeBuild: True, NodeIndexSize: 100"));
    EXPECT_EQ(0, cfg->GetSentenceFromParagraph(config::ParagraphType::GROUP, 3).compare("Group: POS_META, NodeBuild: False, NodeRun: On, NodeSamplingRatio: 100"));
    EXPECT_EQ(0, cfg->GetSentenceFromParagraph(config::ParagraphType::GROUP, 5).compare(""));
    EXPECT_EQ(0, cfg->GetSentenceFromParagraph(config::ParagraphType::GROUP, 99).compare(""));

    EXPECT_EQ(0, cfg->GetSentenceFromParagraph(config::ParagraphType::FILTER, 0).compare("Filter: Basic, Item: (BI_0, BI_1, BI_2)"));
    EXPECT_EQ(0, cfg->GetSentenceFromParagraph(config::ParagraphType::FILTER, 1).compare("Filter: Range, Item: (AIR_0 ... AIR_10)"));

    EXPECT_EQ(0, cfg->GetSentenceFromParagraph(config::ParagraphType::NODE, 1).compare("Node: PERF_VOLUME, Type: PERFORMANCE, Build: True, Group: POS, Filter: Basic"));
    EXPECT_EQ(0, cfg->GetSentenceFromParagraph(config::ParagraphType::NODE, 4).compare("Node: PERF_CP, Type: PERFORMANCE, Build: True, Run: On, Filter: Basic, Group: POS_JOURNAL"));
    EXPECT_EQ(0, cfg->GetSentenceFromParagraph(config::ParagraphType::NODE, 9).compare("Node: LAT_CP, Type: LATENCY, Build: False, Run: Off, Group: POS, Filter: Basic"));
    EXPECT_EQ(0, cfg->GetSentenceFromParagraph(config::ParagraphType::NODE, 10).compare("Node: Q_SUBMIT, Type: QUEUE, Group: POS, Filter: Basic"));
    EXPECT_EQ(0, cfg->GetSentenceFromParagraph(config::ParagraphType::NODE, 13).compare("Node: Q_SCHEDULING, Filter: Basic\n, Type: QUEUE, Build   : True, Run : Off, SamplingRatio: 1000,\n          Group: POS"));
}

TEST_F(ConfigParserTest, GetIndexFromParagraph)
{
    EXPECT_EQ(0, cfg->GetIndexFromParagraph(config::ParagraphType::DEFAULT));

    EXPECT_EQ(0, cfg->GetIndexFromParagraph(config::ParagraphType::GROUP, "POS"));
    EXPECT_EQ(1, cfg->GetIndexFromParagraph(config::ParagraphType::GROUP, "POS_JOURNAL"));
    EXPECT_EQ(2, cfg->GetIndexFromParagraph(config::ParagraphType::GROUP, "POS_IO"));
    EXPECT_EQ(3, cfg->GetIndexFromParagraph(config::ParagraphType::GROUP, "POS_META"));
    EXPECT_EQ(4, cfg->GetIndexFromParagraph(config::ParagraphType::GROUP, "POS_RSC"));
    EXPECT_EQ(-1, cfg->GetIndexFromParagraph(config::ParagraphType::GROUP, "INVALIE_NAME"));
    EXPECT_EQ(-1, cfg->GetIndexFromParagraph(config::ParagraphType::GROUP, "PERF_PSD"));

    EXPECT_EQ(0, cfg->GetIndexFromParagraph(config::ParagraphType::FILTER, "Basic"));
    EXPECT_EQ(1, cfg->GetIndexFromParagraph(config::ParagraphType::FILTER, "Range"));
    EXPECT_EQ(-1, cfg->GetIndexFromParagraph(config::ParagraphType::FILTER, "Basiic"));

    EXPECT_EQ(0, cfg->GetIndexFromParagraph(config::ParagraphType::NODE, "PERF_PSD"));
    EXPECT_EQ(1, cfg->GetIndexFromParagraph(config::ParagraphType::NODE, "PERF_VOLUME"));
    EXPECT_EQ(2, cfg->GetIndexFromParagraph(config::ParagraphType::NODE, "PERF_METAFS"));
    EXPECT_EQ(4, cfg->GetIndexFromParagraph(config::ParagraphType::NODE, "PERF_CP"));
    EXPECT_EQ(6, cfg->GetIndexFromParagraph(config::ParagraphType::NODE, "LAT_SUBMIT"));
    EXPECT_EQ(8, cfg->GetIndexFromParagraph(config::ParagraphType::NODE, "LAT_REBUILD"));
    EXPECT_EQ(13, cfg->GetIndexFromParagraph(config::ParagraphType::NODE, "Q_SCHEDULING"));
    EXPECT_EQ(-1, cfg->GetIndexFromParagraph(config::ParagraphType::NODE, "INVALIE_NAME"));
    EXPECT_EQ(-1, cfg->GetIndexFromParagraph(config::ParagraphType::NODE, "POS_RSC"));
}

TEST_F(ConfigParserTest, GetIntValueFromSentence)
{
    // DEFAULT
    EXPECT_EQ(1, cfg->GetIntValueFromSentence("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32", "AirBuild"));
    EXPECT_EQ(1, cfg->GetIntValueFromSentence("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32", "NodeBuild"));
    EXPECT_EQ(0, cfg->GetIntValueFromSentence("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:Off, NodeSamplingRatio: 1000, NodeIndexSize:32", "NodeRun"));
    EXPECT_EQ(3, cfg->GetIntValueFromSentence("StreamingInterval:  3   , AirBuild  : True, NodeBuild:True, NodeRun:Off, NodeSamplingRatio: 1000, NodeIndexSize:32", "StreamingInterval"));
    EXPECT_EQ(1000, cfg->GetIntValueFromSentence("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:off, NodeSamplingRatio: 1000, NodeIndexSize:32", "NodeSamplingRatio"));
    EXPECT_EQ(-1, cfg->GetIntValueFromSentence("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:off, NodeSamplingRatio: 1000, NodeIndexSize:32", "Condition"));
    EXPECT_EQ(32, cfg->GetIntValueFromSentence("StreamingInterval:1, AirBuild  : True, NodeBuild:True, NodeRun:Off, NodeSamplingRatio: 1000, NodeIndexSize:32", "NodeIndexSize"));

    // GROUP
    EXPECT_EQ(0, cfg->GetIntValueFromSentence("Group: POS_RSC, NodeBuild: False, NodeRun: On", "NodeBuild"));
    EXPECT_EQ(1, cfg->GetIntValueFromSentence("Group: POS_RSC, NodeBuild: False, NodeRun: On", "NodeRun"));
    EXPECT_EQ(-1, cfg->GetIntValueFromSentence("Group: POS_RSC, NodeBuild: False, NodeRun: On", "Condition"));

    // FILTER
    EXPECT_EQ(0, cfg->GetIntValueFromSentence("Filter: F1, Item: (AIR_10 ... AIR_32)", "Item", "AIR_10"));
    EXPECT_EQ(9, cfg->GetIntValueFromSentence("Filter: F1, Item: (AIR_10 ... AIR_32)", "Item", "AIR_19"));
    EXPECT_EQ(22, cfg->GetIntValueFromSentence("Filter: F1, Item: (AIR_10 ... AIR_32)", "Item", "AIR_32"));
    EXPECT_EQ(-1, cfg->GetIntValueFromSentence("Filter: F1, Item: (AIR_10 ... AIR_32)", "Item", "AIR_33"));
    EXPECT_EQ(0, cfg->GetIntValueFromSentence("Filter: F1, Item: (AIR_READ, AIR_WRITE)", "Item", "AIR_READ"));
    EXPECT_EQ(1, cfg->GetIntValueFromSentence("Filter: F1, Item: (AIR_READ, AIR_WRITE)", "Item", "AIR_WRITE"));
    EXPECT_EQ(-1, cfg->GetIntValueFromSentence("Filter: F1, Item: (AIR_READ, AIR_WRITE)", "Item", "AIR_WRITEE"));
    EXPECT_EQ(0, cfg->GetIntValueFromSentence("Filter: F1, Item: (AIR_ONE)", "Item", "AIR_ONE"));

    // NODE
    EXPECT_EQ(1, cfg->GetIntValueFromSentence("Node: Q_IOWORER, Type: QUEUE, Build: True, Filter: Basic, Run: Off, SamplingRatio: 10", "Build"));
    EXPECT_EQ(0, cfg->GetIntValueFromSentence("Node: Q_IOWORER, Type: QUEUE, Build: True, Filter: Basic, Run: Off, SamplingRatio: 10", "Run"));
    EXPECT_EQ(-1, cfg->GetIntValueFromSentence("Node: Q_IOWORER, Type: QUEUE, Build: True, Filter: Basic, Run: Off, SamplingRatio: 10", "AirBuild"));
    EXPECT_EQ(10, cfg->GetIntValueFromSentence("Node: Q_IOWORER, Type: QUEUE, Build: True, Filter: Basic, Run: Off, SamplingRatio: 10", "SamplingRatio"));
}

TEST_F(ConfigParserTest, GetItemStrFromFilterSentence)
{
    EXPECT_EQ(0, cfg->GetItemStrFromFilterSentence("Filter: F1, Item: (AIR_10 ... AIR_32)", 0).compare("AIR_10"));
    EXPECT_EQ(0, cfg->GetItemStrFromFilterSentence("Filter: F1, Item: (AIR_10 ... AIR_32)", 9).compare("AIR_19"));
    EXPECT_EQ(0, cfg->GetItemStrFromFilterSentence("Filter: F1, Item: (AIR_10 ... AIR_32)", 22).compare("AIR_32"));
    EXPECT_EQ(0, cfg->GetItemStrFromFilterSentence("Filter: F1, Item: (AIR_READ, AIR_WRITE)", 0).compare("AIR_READ"));
    EXPECT_EQ(0, cfg->GetItemStrFromFilterSentence("Filter: F1, Item: (AIR_READ, AIR_WRITE)", 1).compare("AIR_WRITE"));
    EXPECT_EQ(0, cfg->GetItemStrFromFilterSentence("Filter: F1, Item: (AIR_ONE)", 0).compare("AIR_ONE"));
}

TEST_F(ConfigParserTest, GetItemSizeFromFilterSentence)
{
    EXPECT_EQ(13, cfg->GetItemSizeFromFilterSentence("Filter: F1, Item: (AIR_0 ... AIR_12)"));
    EXPECT_EQ(4, cfg->GetItemSizeFromFilterSentence("Filter: F1, Item: (AIR_0, AIR_1, AIR_2, AIR_8)"));
    EXPECT_EQ(1, cfg->GetItemSizeFromFilterSentence("Filter: F1, Item: (AIR_ONE)"));
}

TEST_F(ConfigParserTest, GetStrValueFromSentence)
{
    // DEFAULT
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32", "AirBuild").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32", "NodeBuild").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:Off, NodeSamplingRatio: 1000, NodeIndexSize:32", "StreamingInterval").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:Off, NodeSamplingRatio: 1000, NodeIndexSize:32", "SamplingRatio").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:Off, NodeSamplingRatio: 1000, NodeIndexSize:32", "Condition").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:off, NodeSamplingRatio: 1000, NodeIndexSize:32", "NodeRun").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:off, NodeSamplingRatio: 1000, NodeIndexSize:32", "NodeIndexSize").compare(""));

    // GROUP
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Group: POS_RSC  , NodeBuild: False, NodeRun: On", "Group").compare("POS_RSC"));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Group: POS_RSC", "Group").compare("POS_RSC"));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Group: POS_RSC  , NodeBuild: False, NodeRun: On", "NodeRun").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Group: POS_RSC  , NodeBuild: False, NodeRun: On", "NodeBuild").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Group: POS_RSC  , NodeBuild: False, NodeRun: On", "NodeRun").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Group: POS_RSC  , NodeBuild: False, NodeRun: On", "SamplingRatio").compare(""));

    // FILTER
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Filter: F1, Item: (AIR_9 ... AIR_32)", "Filter").compare("F1"));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Filter: F2, Item: (AIR_12, AIR_34, AIR_39)", "Filter").compare("F2"));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Filter: F3, Item: (AIR_ONE)", "Filter").compare("F3"));

    // NODE
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Node:   Q_IOWORKER    , Type: QUEUE, Build: True, Run: Off, SamplingRatio: 10", "Node").compare("Q_IOWORKER"));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Node: Q_IOWORKER, Type: QUEUE, Build: True, Run: Off, SamplingRatio: 10", "Type").compare("QUEUE"));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Node: Q_IOWORKER, Type: QUEUE, Build: True, Run: Off, SamplingRatio: 10, Group:  IO     ", "Group").compare("IO"));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Node: Q_IOWORKER, Type: QUEUE, Build: True, Run: Off, SamplingRatio: 10", "Run").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Node: Q_IOWORKER, Type: QUEUE, Build: True, Run: Off, SamplingRatio: 10", "SamplingRatio").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Node: Q_IOWORKER, Type: QUEUE, Build: True, Run: Off, SamplingRatio: 10", "AirBuild").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Node: Q_IOWORKER, Type: QUEUE, Build: True, Run: Off, SamplingRatio: 10", "Build").compare(""));
    EXPECT_EQ(0, cfg->GetStrValueFromSentence("Node: Q_IOWORKER, Type: QUEUE, Build: True, Run: Off, SamplingRatio: 10", "Group").compare(""));
}

TEST_F(ConfigParserTest, GetSentenceCount)
{
    EXPECT_EQ(1, cfg->GetSentenceCount(config::ParagraphType::DEFAULT));
    EXPECT_EQ(5, cfg->GetSentenceCount(config::ParagraphType::GROUP));
    EXPECT_EQ(2, cfg->GetSentenceCount(config::ParagraphType::FILTER));
    EXPECT_EQ(15, cfg->GetSentenceCount(config::ParagraphType::NODE));
}

TEST_F(ConfigInterfaceTest, GetSentenceName)
{
    EXPECT_EQ(0, cfg::GetSentenceName(config::ParagraphType::DEFAULT, 0).compare(""));
    EXPECT_EQ(0, cfg::GetSentenceName(config::ParagraphType::GROUP, 3).compare("POS_META"));
    EXPECT_EQ(0, cfg::GetSentenceName(config::ParagraphType::FILTER, 1).compare("Range"));
    EXPECT_EQ(0, cfg::GetSentenceName(config::ParagraphType::NODE, 0).compare("PERF_PSD"));
    EXPECT_EQ(0, cfg::GetSentenceName(config::ParagraphType::NODE, 1).compare("PERF_VOLUME"));
}

TEST_F(ConfigInterfaceTest, GetSentenceIndex)
{
    EXPECT_EQ(2, cfg::GetSentenceIndex(config::ParagraphType::GROUP, "POS_IO"));
    EXPECT_EQ(0, cfg::GetSentenceIndex(config::ParagraphType::FILTER, "Basic"));
    EXPECT_EQ(0, cfg::GetSentenceIndex(config::ParagraphType::NODE, "PERF_PSD"));
    EXPECT_EQ(4, cfg::GetSentenceIndex(config::ParagraphType::NODE, "PERF_CP"));
    EXPECT_EQ(8, cfg::GetSentenceIndex(config::ParagraphType::NODE, "LAT_REBUILD"));
    EXPECT_EQ(13, cfg::GetSentenceIndex(config::ParagraphType::NODE, "Q_SCHEDULING"));
    EXPECT_EQ(-1, cfg::GetSentenceIndex(config::ParagraphType::NODE, "UNKNOWN"));
}

TEST_F(ConfigInterfaceTest, GetIntValue)
{
    EXPECT_EQ(1, cfg::GetIntValue(config::ParagraphType::DEFAULT, "StreamingInterval"));
    EXPECT_EQ(1000, cfg::GetIntValue(config::ParagraphType::DEFAULT, "NodeSamplingRatio"));
    EXPECT_EQ(32, cfg::GetIntValue(config::ParagraphType::DEFAULT, "NodeIndexSize"));
    EXPECT_EQ(1, cfg::GetIntValue(config::ParagraphType::DEFAULT, "AirBuild"));
    EXPECT_EQ(1, cfg::GetIntValue(config::ParagraphType::DEFAULT, "NodeBuild"));
    EXPECT_EQ(1, cfg::GetIntValue(config::ParagraphType::DEFAULT, "NodeRun"));

    EXPECT_EQ(0, cfg::GetIntValue(config::ParagraphType::GROUP, "NodeBuild", "POS_META"));
    EXPECT_EQ(1, cfg::GetIntValue(config::ParagraphType::GROUP, "NodeBuild", "POS"));
    EXPECT_EQ(32, cfg::GetIntValue(config::ParagraphType::GROUP, "NodeIndexSize", "POS"));
    EXPECT_EQ(100, cfg::GetIntValue(config::ParagraphType::GROUP, "NodeIndexSize", "POS_JOURNAL"));

    EXPECT_EQ(0, cfg::GetIntValue(config::ParagraphType::FILTER, "Item", "Basic", "BI_0"));
    EXPECT_EQ(1, cfg::GetIntValue(config::ParagraphType::FILTER, "Item", "Basic", "BI_1"));
    EXPECT_EQ(2, cfg::GetIntValue(config::ParagraphType::FILTER, "Item", "Basic", "BI_2"));
    EXPECT_EQ(0, cfg::GetIntValue(config::ParagraphType::FILTER, "Item", "Range", "AIR_0"));
    EXPECT_EQ(3, cfg::GetIntValue(config::ParagraphType::FILTER, "Item", "Range", "AIR_3"));
    EXPECT_EQ(7, cfg::GetIntValue(config::ParagraphType::FILTER, "Item", "Range", "AIR_7"));
    EXPECT_EQ(10, cfg::GetIntValue(config::ParagraphType::FILTER, "Item", "Range", "AIR_10"));

    EXPECT_EQ(1, cfg::GetIntValue(config::ParagraphType::NODE, "Build", "PERF_PSD"));
    EXPECT_EQ(1, cfg::GetIntValue(config::ParagraphType::NODE, "Run", "PERF_PSD"));
    EXPECT_EQ(0, cfg::GetIntValue(config::ParagraphType::NODE, "Build", "LAT_PSD"));
    EXPECT_EQ(100, cfg::GetIntValue(config::ParagraphType::NODE, "SamplingRatio", "LAT_PSD"));
    EXPECT_EQ(1000, cfg::GetIntValue(config::ParagraphType::NODE, "SamplingRatio", "LAT_SUBMIT"));
    EXPECT_EQ(32, cfg::GetIntValue(config::ParagraphType::NODE, "IndexSize", "PERF_REBUILD"));
    EXPECT_EQ(100, cfg::GetIntValue(config::ParagraphType::NODE, "IndexSize", "PERF_CP"));
    EXPECT_EQ(77, cfg::GetIntValue(config::ParagraphType::NODE, "IndexSize", "LAT_PSD"));
}

TEST_F(ConfigInterfaceTest, GetStrValue)
{
    EXPECT_EQ(0, cfg::GetStrValue(config::ParagraphType::NODE, "Type", "PERF_PSD").compare("PERFORMANCE"));
}

TEST_F(ConfigInterfaceTest, GetItemStrWithFilterName)
{
    EXPECT_EQ(0, cfg::GetItemStrWithFilterName("Basic", 0).compare("BI_0"));
    EXPECT_EQ(0, cfg::GetItemStrWithFilterName("Basic", 1).compare("BI_1"));
    EXPECT_EQ(0, cfg::GetItemStrWithFilterName("Basic", 2).compare("BI_2"));
    EXPECT_EQ(0, cfg::GetItemStrWithFilterName("Range", 4).compare("AIR_4"));
    EXPECT_EQ(0, cfg::GetItemStrWithFilterName("Range", 8).compare("AIR_8"));
    EXPECT_EQ(0, cfg::GetItemStrWithFilterName("Range", 9).compare("AIR_9"));
}

TEST_F(ConfigInterfaceTest, GetItemSizeWithFilterName)
{
    EXPECT_EQ(3, cfg::GetItemSizeWithFilterName("Basic"));
    EXPECT_EQ(11, cfg::GetItemSizeWithFilterName("Range"));
}

TEST_F(ConfigCheckerTest, CheckParagraphRule_DefaultType)
{
    constexpr air::string_view default_paragraph = R"DEFAULT(
    "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"
    )DEFAULT";
    constexpr air::string_view default_paragraph_with_comment = R"DEFAULT(
            // default setting for Mandatory
    "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"
    )DEFAULT";
    constexpr air::string_view default_paragraph_multi_line = R"DEFAULT(
            // default setting for Mandatory
    "StreamingInterval:1, AirBuild:True, 
        NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"
    )DEFAULT";
    constexpr air::string_view default_paragraph_multi_line2 = R"DEFAULT(
            // default setting for Mandatory
    "StreamingInterval:1, AirBuild:True
        ,NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"
    )DEFAULT";

    EXPECT_EQ(0, cfg_checker->CheckParagraphRule(config::ParagraphType::DEFAULT, default_paragraph));
    EXPECT_EQ(0, cfg_checker->CheckParagraphRule(config::ParagraphType::DEFAULT, default_paragraph_with_comment));
    EXPECT_EQ(0, cfg_checker->CheckParagraphRule(config::ParagraphType::DEFAULT, default_paragraph_multi_line));
    EXPECT_EQ(0, cfg_checker->CheckParagraphRule(config::ParagraphType::DEFAULT, default_paragraph_multi_line2));
}

TEST_F(ConfigCheckerTest, CheckParagraphRule_GroupType)
{
    constexpr air::string_view group_paragraph = R"GROUP(
    "Group:POS_IO"
    "Group:POS_META"
    "Group: testetsetsetset, NodeRun: On, NodeBuild: False"
    "Group: POS_REBUILD, NodeSamplingRatio: 1000, NodeBuild: True"
    )GROUP";
    constexpr air::string_view group_paragraph_with_comment = R"GROUP(
    "Group:POS_IO"
    "Group:POS_META"// comment test
    "Group: testetsetsetset, NodeRun: On, NodeBuild: False"
    "Group: POS_REBUILD, NodeSamplingRatio: 1000, NodeBuild: True"
    )GROUP";
    constexpr air::string_view group_paragraph_multi_line = R"GROUP(
            // group setting, TBD
    "Group:POS_IO"
    "Group:POS_META"// comment test
    "Group: testetsetsetset, NodeRun: On, NodeBuild: False"
    "Group: POS_REBUILD
        , NodeSamplingRatio: 1000, NodeBuild: True"
    )GROUP";
    EXPECT_EQ(0, cfg_checker->CheckParagraphRule(config::ParagraphType::GROUP, group_paragraph));
    EXPECT_EQ(0, cfg_checker->CheckParagraphRule(config::ParagraphType::GROUP, group_paragraph_with_comment));
    EXPECT_EQ(0, cfg_checker->CheckParagraphRule(config::ParagraphType::GROUP, group_paragraph_multi_line));
}

TEST_F(ConfigCheckerTest, CheckParagraphRule_FilterType)
{
    constexpr air::string_view filter_paragraph1 = R"FILTER("Filter:F1, Item: (F1_0)")FILTER";
    constexpr air::string_view filter_paragraph2 = R"FILTER("Filter:_F1, Item: (F1_0,F1_1,F1_2) ")FILTER";
    constexpr air::string_view filter_paragraph3 = R"FILTER("Filter:F1_, Item: (F1_0...F1_13)   ")FILTER";
    constexpr air::string_view filter_paragraph_error1 = R"FILTER("Filter: F1, Item: (F1_0, F1_1),")FILTER";
    constexpr air::string_view filter_paragraph_error2 = R"FILTER("Filter: F1, Item: (F1_0, F1_1")FILTER";
    constexpr air::string_view filter_paragraph_error3 = R"FILTER("Filter: F1, Item: F1_0, F1_1)")FILTER";
    constexpr air::string_view filter_paragraph_error4 = R"FILTER("Filter: F1, Item: ((F1_0, F1_1)")FILTER";
    constexpr air::string_view filter_paragraph_error5 = R"FILTER("Filter: F1, Item: (F1_0, F1_1))")FILTER";
    constexpr air::string_view filter_paragraph_error6 = R"FILTER(": F1, Item: (F1_0, F1_1)")FILTER";
    constexpr air::string_view filter_paragraph_error7 = R"FILTER("Filter: , Item: (F1_0, F1_1)")FILTER";
    constexpr air::string_view filter_paragraph_error8 = R"FILTER("Filter: F1, Item: ")FILTER";
    constexpr air::string_view filter_paragraph_error9 = R"FILTER("Filter: F1, Item: ()")FILTER";

    EXPECT_EQ(0, cfg_checker->CheckParagraphRule(config::ParagraphType::FILTER, filter_paragraph1));
    EXPECT_EQ(0, cfg_checker->CheckParagraphRule(config::ParagraphType::FILTER, filter_paragraph2));
    EXPECT_EQ(0, cfg_checker->CheckParagraphRule(config::ParagraphType::FILTER, filter_paragraph3));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::FILTER, filter_paragraph_error1));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::FILTER, filter_paragraph_error2));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::FILTER, filter_paragraph_error3));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::FILTER, filter_paragraph_error4));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::FILTER, filter_paragraph_error5));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::FILTER, filter_paragraph_error6));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::FILTER, filter_paragraph_error7));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::FILTER, filter_paragraph_error8));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::FILTER, filter_paragraph_error9));
}

TEST_F(ConfigCheckerTest, CheckParagraphRule_NodeType)
{
    constexpr air::string_view node_paragraph = R"NODE(
    "Node: Q_SUBMIT, Type:Queue, Build:True"
    "Node:Q_EVENT , Type:Queue, Build:False"
    "Node:PERF_VOLUME, Type:Performance, Build:True"
    "Node: LAT_VOLUME  , Type:Latency, Build:True"
    "Node: Q_REACTOR"
    )NODE";
    constexpr air::string_view node_paragraph_with_comment = R"NODE(
            // node setting
    "Node: Q_SUBMIT, Type:Queue, Build:True"
    // this comment is okay
    "Node:Q_EVENT , Type:Queue, Build:False"
    // but do not insert comment within double quotation marks!
    "Node:PERF_VOLUME, Type:Performance, Build:True"
    "Node: LAT_VOLUME  , Type:Latency, Build:True"
    "Node: Q_REACTOR"   // valid comment
    )NODE";
    constexpr air::string_view node_paragraph_multi_line = R"NODE(
            // node setting
    "Node: Q_SUBMIT, Type:Queue, Build:True"
    // this comment is okay
    "Node:Q_EVENT , Type:Queue, Build:False"
    // but do not insert comment within double quotation marks!
    "Node:PERF_VOLUME,
         
        Type:Performance, 
        Build:True"
    "Node: LAT_VOLUME  , Type:Latency, Build:True"
    "Node: Q_REACTOR"
    )NODE";
    constexpr air::string_view node_paragraph_multi_line2 = R"NODE(
            // node setting
    "Node: Q_SUBMIT, Type:Queue, Build:True"
    // this comment is okay
    "Node:Q_EVENT     , Type:Queue, Build:False"
    // but do not insert comment within double quotation marks!
    "Node:PERF_VOLUME           
         
        ,Type:Performance, 
        Build:True"
    "Node: LAT_VOLUME  , Type:Latency, Build:True"
    "Node: Q_REACTOR"
    )NODE";

    EXPECT_EQ(0, cfg_checker->CheckParagraphRule(config::ParagraphType::NODE, node_paragraph));
    EXPECT_EQ(0, cfg_checker->CheckParagraphRule(config::ParagraphType::NODE, node_paragraph_with_comment));
    EXPECT_EQ(0, cfg_checker->CheckParagraphRule(config::ParagraphType::NODE, node_paragraph_multi_line));
    EXPECT_EQ(0, cfg_checker->CheckParagraphRule(config::ParagraphType::NODE, node_paragraph_multi_line2));
}

TEST_F(ConfigCheckerTest, SentenceFormatViolation)
{
    constexpr air::string_view default_no_quote = R"DEFAULT(
    StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32
    )DEFAULT";
    constexpr air::string_view default_no_key_viol = R"DEFAULT(
    "StreamingInterval:1, :True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"
    )DEFAULT";
    constexpr air::string_view default_no_value_viol = R"DEFAULT(
    "StreamingInterval:1, AirBuild:, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"
    )DEFAULT";
    constexpr air::string_view default_no_colon_viol = R"DEFAULT(
    "StreamingInterval:1, AirBuild True, NodeBuild True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"
    )DEFAULT";
    constexpr air::string_view default_no_colon_viol2 = R"DEFAULT(
    "StreamingInterval 1, AirBuild True, NodeBuild True, NodeRun On, NodeSamplingRatio 1000, NodeIndexSize 32"
    )DEFAULT";
    constexpr air::string_view default_comment_viol = R"DEFAULT(
    "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On,   // comment violation
        NodeSamplingRatio: 1000, NodeIndexSize:32"
    )DEFAULT";
    constexpr air::string_view default_no_end_quote = R"DEFAULT(
    "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On,
        NodeSamplingRatio: 1000, NodeIndexSize:32
    )DEFAULT";
    constexpr air::string_view default_comma_count_viol = R"DEFAULT(
    "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On,
        NodeSamplingRatio: 1000, NodeIndexSize:32, "
    )DEFAULT";

    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::DEFAULT, default_no_quote));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::DEFAULT, default_no_key_viol));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::DEFAULT, default_no_value_viol));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::DEFAULT, default_no_colon_viol));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::DEFAULT, default_no_colon_viol2));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::DEFAULT, default_comment_viol));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::DEFAULT, default_no_end_quote));
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::DEFAULT, default_comma_count_viol));
}

TEST_F(ConfigCheckerTest, NameDuplicatedViolation)
{
    constexpr air::string_view group_key_duplicated(R"GROUP(
    "Group:POS_IO, Build:False"
    "Group:POS_REBUILD"
    "Group: testetsetsetset, NodeRun: on, NodeBuild: False"
    "Group: POS_REBUILD, NodeSamplingRatio: 1000, NodeBuild: True"
    )GROUP");
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::GROUP, group_key_duplicated));

    constexpr air::string_view filter_key_duplicated(R"FILTER(
    "Filter:F1, Item:(F1_0, F1_1)"
    "Filter:F1, Item:(F2_0 ... F2_3)"
    )FILTER");
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::FILTER, filter_key_duplicated));

    constexpr air::string_view node_key_duplicated = R"NODE(
    "Node: Q_SUBMIT, Type:Queue, Build:True"
    "Node:Q_EVENT , Type:Queue, Build:False"
    "Node:PERF_VOLUME, Type:Performance, Build:True"
    "Node: LAT_VOLUME  , Type:Latency, Build:True"
    "Node: Q_EVENT"
    )NODE";
    EXPECT_ANY_THROW(cfg_checker->CheckParagraphRule(config::ParagraphType::NODE, node_key_duplicated));
}

TEST_F(ConfigCheckerTest, CheckKeyRule_DefaultType)
{
    // DEFAULT
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "NodeSamplingRatio: 1000, NodeRun:On, NodeBuild:True, AirBuild:True, StreamingInterval:1, NodeIndexSize:32"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "NodeSamplingRatio: 1000, NodeRun:On, NodeBuild:True, AirBuild:True, StreamingInterval:1, NodeIndexSize : 32"));
}

TEST_F(ConfigCheckerTest, CheckKeyRule_GroupType)
{
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "Group: POS_RSC, NodeBuild: False, NodeRun: On, NodeSamplingRatio: 10"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "NodeBuild: False, Group: POS_RSC, NodeSamplingRatio: 10, NodeRun: On"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "NodeBuild: False, NodeRun: On, NodeSamplingRatio: 10, Group: POS_RSC"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "Group: POS_RSC, NodeBuild: False, NodeRun: On, NodeSamplingRatio: 100"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "Group: POS_RSC, NodeBuild: False, NodeRun: On"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "Group: POS_RSC, NodeBuild: False"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "Group: POS_RSC"));
}

TEST_F(ConfigCheckerTest, CheckKeyRule_FilterType)
{
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::FILTER, "Filter: F1, Item: (F1_0, F1_1)"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::FILTER, "Filter: F1, Item: (F1_0 ... F1_1)"));
}

TEST_F(ConfigCheckerTest, CheckKeyRule_NodeType)
{
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE, Build: True, Run: On, Group: POS_Journal, Filter: Basic"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Type: PERFORMANCE, Group: POS_Journal, Run: On, Build: True, Node: PERF_CP, Filter: Basic"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE, Build: True, Filter: Basic, Run: On, Group: UNGROUPED"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE,Filter:Basic, Build: True, Run: On, Group: UNGROUPED"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE, Build: True, Run: On, Group: UNGROUPED, Filter: Basic"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE, Filter: Basic, Build: True, Group: UNGROUPED"));
    EXPECT_EQ(0, cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Filter: Basic, Node: PERF_CP, Type: PERFORMANCE, Group: UNGROUPED"));
}

TEST_F(ConfigCheckerTest, KeyTypoViolation_DefaultType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AAABuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, N0deBuild:True, NodeRun:On, NodeSamplingRati0: 1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NR:On, NodeSamplingRatio: 1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "SI:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NSR: 1333, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, AS:33"));
}

TEST_F(ConfigCheckerTest, KeyTypoViolation_GroupType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "Group: POS_RSC, NB: False, NodeRun: On, NodeSamplingRatio: 10"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "Group: POS_RSC, NodeBuild: False, RN: On, NodeSamplingRatio: 10"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "Group: POS_RSC, NodeBuild: False, NodeRun: On, NSR: 10"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "Group: POS_RSC, NodeBuild: False, Node: On, NodeSamplingRatio: 10"));
}

TEST_F(ConfigCheckerTest, KeyTypoViolation_FilterType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::FILTER, "Filte: F1, Item: (AIR_BASE)"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::FILTER, "Filter: F1, Itemm: (AIR_BASE)"));
}

TEST_F(ConfigCheckerTest, KeyTypoViolation_NodeType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "NN: PERF_CP, Type: PERFORMANCE, Build: True, Run: On, Group: POS_Journal, SamplingRatio: 100"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: PERF_CP, t: PERFORMANCE, Build: True, Run: On, Group: POS_Journal, SamplingRatio: 100"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE, B: True, Run: On, Group: POS_Journal, SamplingRatio: 100"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE, Build: True, RN: On, Group: POS_Journal, SamplingRatio: 100"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE, Build: True, Run: On, Group: POS_Journal, SamplingRatio: 100"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE, Build: True, Run: On, GN: POS_Journal, SamplingRatio: 100"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE, Build: True, Run: On, Group: POS_Journal, SR: 100"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE, SamplingRatio: 10"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: LAT_TEST, Type: LATENCY, SamplingRatio : 1000"));
}

TEST_F(ConfigCheckerTest, KeyMandatoryViolation_DefaultType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeSamplingRatio:1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeRun:On, NodeSamplingRatio:1000, NodeIndexSize: 50"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, NodeBuild:True, NodeRun:On, NodeSamplingRatio:1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio:1000, NodeIndexSize:10"));
}

TEST_F(ConfigCheckerTest, KeyMandatoryViolation_GroupType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "NodeBuild: False, NodeRun: On, NodeSamplingRatio: 10"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "NodeBuild: False, NodeSamplingRatio: 10"));
}

TEST_F(ConfigCheckerTest, KeyMandatoryViolation_FilterType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::FILTER, "Filter: F1"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::FILTER, "Item: (AIR_0)"));
}

TEST_F(ConfigCheckerTest, KeyMandatoryViolation_NodeType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: PERF_CP"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Type: PERFORMANCE"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Build: True, Run: On, Type: PERFORMANCE, Node: Test1, Filter: F2"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Run: On, Node: PERF_CP, Type: QUEUE, Group: G2"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Run: On"));
}

TEST_F(ConfigCheckerTest, KeyDuplicationViolation_DefaultType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, StreamingInterval: 10, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, AirBuild: False, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeBuild:True, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeRun:On, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "NodeSamplingRatio: 10, StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::DEFAULT, "NodeSamplingRatio: 10, NodeIndexSize: 100, StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeIndexSize: 10"));
}

TEST_F(ConfigCheckerTest, KeyDuplicationViolation_GroupType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "Group: POS_RSC, NodeBuild: False, NodeRun: On, NodeSamplingRatio: 10, Group: POS_RSC"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "Group: POS_RSC, NodeBuild: False, NodeRun: On, NodeSamplingRatio: 10, NodeBuild: False"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "Group: POS_RSC, NodeBuild: False, NodeRun: On, NodeSamplingRatio: 10, NodeRun: On"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::GROUP, "Group: POS_RSC, NodeBuild: False, NodeRun: On, NodeSamplingRatio: 10, NodeSamplingRatio: 1"));
}

TEST_F(ConfigCheckerTest, KeyDuplicationViolation_FilterType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::FILTER, "Filter: F1, Item: (AIR_0), Filter: F2"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::FILTER, "Filter: F1, Item: (AIR_0), Item: (AIR_2)"));
}

TEST_F(ConfigCheckerTest, KeyDuplicationViolation_NodeType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: LAT_CP, Type: LATENCY, Build: True, Run: On, Group: POS_Journal, Node: LAT_CP"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: LAT_CP, Type: LATENCY, Type: LATENCY, Build: True, Run: On, Group: POS_Journal"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: LAT_CP, Type: LATENCY, Build: True, Build: True, Run: On, Group: POS_Journal"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: LAT_CP, Type: LATENCY, Build: True, Run: On, Run: On, Group: POS_Journal"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: LAT_CP, Type: LATENCY, Build: True, Run: On, Group: POS_Journal, Filter: F1, Filter: F2"));
    EXPECT_ANY_THROW(cfg_checker->CheckKeyRule(config::ParagraphType::NODE, "Node: LAT_CP, Type: LATENCY, Build: True, Run: On, Group: POS_Journal, Group: POS_Journal"));
}

TEST_F(ConfigCheckerTest, CheckValueRulePass)
{
    EXPECT_EQ(0, cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize: 30"));
    EXPECT_EQ(0, cfg_checker->CheckValueRule(config::ParagraphType::GROUP, "Group: POS_RSC, NodeBuild: False, NodeRun: On, NodeSamplingRatio: 100"));
    EXPECT_EQ(0, cfg_checker->CheckValueRule(config::ParagraphType::GROUP, "Group: POS_RSC"));
    EXPECT_EQ(0, cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F1, Item: (BASE_0 , BASE1)"));
    EXPECT_EQ(0, cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F2, Item: (BASE_0 ... BASE_3)"));
    EXPECT_EQ(0, cfg_checker->CheckValueRule(config::ParagraphType::NODE, "Node: LAT_CP, Type: LATENCY, Build: True, Run: On, Group: POS, Filter: Basic"));
}

TEST_F(ConfigCheckerTest, ValueVaildityViolation_DefaultType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:199, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval: b , AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval: 1a, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval: aaaaaa, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild: 100, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:-99, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:abcd, NodeSamplingRatio: 1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:abcd, NodeSamplingRatio: 1000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:abcd, NodeSamplingRatio: 10000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: -1, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 0.0000000000000001, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 0.a001, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1ra01, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize: -10"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 100000, NodeIndexSize:32"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize: 10000000000"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize: 5000000000"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::DEFAULT, "StreamingInterval:1, AirBuild:True, NodeBuild:True, NodeRun:On, NodeSamplingRatio: 1000, NodeIndexSize: 4294967296"));
}

TEST_F(ConfigCheckerTest, ValueVaildityViolation_GroupType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::GROUP, "Group: CANNOT_EXCEED_30_CHARACTERS_IN_STRING_TYPE_DATA, NodeBuild: False, NodeRun: On, NodeSamplingRatio: 10"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::GROUP, "Group: POS_RSC, NodeBuild: 123, NodeRun: On, NodeSamplingRatio: 10"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::GROUP, "Group: POS_RSC, NodeBuild: False, NodeRun: 123, NodeSamplingRatio: 10"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::GROUP, "Group: POS_RSC, NodeBuild: False, NodeRun: On, NodeSamplingRatio: 0"));
}

TEST_F(ConfigCheckerTest, ValueVaildityViolation_FilterType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: CANNOT_EXCEED_30_CHARACTERS_IN_STRING_TYPE_DATA, Item: (F1_AAA, F1_BBB)"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F2, Item: (F2_0... F2_3), Run: False"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F3, Item: (F3_0, ... F3_2)"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F4, Item: (F4_0 F4_1)"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F4, Item: (.)"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F4, Item: (..)"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F4, Item: ()"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F4, Item: (     )"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F5, Item: (F5_0 ... )"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F5, Item: ( ... F5_3 )"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F5, Item: (F5_1 ... F5)"));     // underscope(_) missing
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F5, Item: (F5 ... F5_3)"));     // underscope(_) missing
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F5, Item: (F5_3 ... F55_6)"));  // prefix has to be same
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F5, Item: (F5_a3 ... F5_3)"));  // suffix has to be integer
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F5, Item: (F5_1 ... F5_-3)"));  // suffix has to be integer
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F5, Item: (F5_13 ... F5_3)"));  // suffix underflow
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F5, Item: (F5_42 ... F5_42)")); // suffix underflow
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F5, Item: (F5_0 ... F5_100)")); //max size: 100 (suffix overflow)
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F6, Item: (, F6_1)"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F6, Item: (F6_0, F6_1 F6_2)"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F6, Item: (F6_0,, F6_1)"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::FILTER, "Filter: F6, Item: (F6_0, F6_1 F6_2, )"));
}

TEST_F(ConfigCheckerTest, ValueVaildityViolation_NodeType)
{
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::NODE, "Node: CANNOT_EXCEED_30_CHARACTERS_IN_STRING_TYPE_DATA, Type: PERFORMANCE, Build: True, Run: On, SamplingRatio:1000, Group: POS_JOURNAL, Filter: Basic"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: NOTYPE, Build: True, Run: On, SamplingRatio:1000, Group: POS_JOURNAL"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE, Build: 123, Run: On, SamplingRatio:1000, Group: POS_JOURNAL"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE, Build: True, Run: 123, SamplingRatio:1000, Group: POS_JOURNAL"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE, Build: True, Run: On, SamplingRatio:0.9, Group: POS_JOURNAL"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE, Build: True, Run: On, SamplingRatio:1000, Group: POS_JOURNALLLL"));
    EXPECT_ANY_THROW(cfg_checker->CheckValueRule(config::ParagraphType::NODE, "Node: PERF_CP, Type: PERFORMANCE, Build: True, Run: On, SamplingRatio:1000, Group: POS_JOURNAL, Filter: Basiccc"));
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
