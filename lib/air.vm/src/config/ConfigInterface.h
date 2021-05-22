
#ifndef AIR_CONFIG_INTERFACE_H
#define AIR_CONFIG_INTERFACE_H

#include <string>

#include "src/config/ConfigLib.h"
#include "src/config/ConfigParser.h"
#include "src/lib/StringView.h"

namespace config
{
class ConfigInterface
{
public:
    ConfigInterface(void)
    {
    }
    virtual ~ConfigInterface(void)
    {
    }

    static constexpr int32_t
    GetSentenceIndex(ParagraphType type, air::string_view name = "")
    {
        return config_parser.GetIndexFromParagraph(type, name);
    }

    static constexpr uint32_t
    GetSentenceCount(ParagraphType type)
    {
        return config_parser.GetSentenceCount(type);
    }

    static constexpr air::string_view
    GetSentenceName(ParagraphType type, uint32_t index)
    {
        air::string_view key{""};
        if (ParagraphType::GROUP == type)
        {
            key = "Group";
        }
        else if (ParagraphType::NODE == type)
        {
            key = "Node";
        }
        else if (ParagraphType::FILTER == type)
        {
            key = "Filter";
        }
        else
        {
            return "";
        }

        air::string_view sentence = config_parser.GetSentenceFromParagraph(type, index);
        return config_parser.GetStrValueFromSentence(sentence, key);
    }

    static constexpr int32_t
    GetIntValue(ParagraphType type, air::string_view key, int index_c, air::string_view item)
    {
        if (ParagraphType::DEFAULT == type)
        {
            if (key != "AirBuild" && key != "StreamingInterval" && key != "NodeBuild" && key != "NodeRun" &&
                key != "NodeSamplingRatio" && key != "NodeIndexSize" && key != "NodeEnumSize")
            {
                return -1;
            }
        }
        else if (ParagraphType::GROUP == type)
        {
            if (key != "NodeBuild" && key != "NodeRun" && key != "NodeSamplingRatio" && key != "NodeIndexSize" && key != "NodeEnumSize")
            {
                return -1;
            }
        }
        else if (ParagraphType::FILTER == type)
        {
            if (key != "Item")
            {
                return -1;
            }
        }
        else if (ParagraphType::NODE == type)
        {
            if (key != "Build" && key != "Run" && key != "SamplingRatio" && key != "IndexSize")
            {
                return -1;
            }
        }

        int32_t index = index_c;
        air::string_view sentence = config_parser.GetSentenceFromParagraph(type, index);
        int32_t ret = config_parser.GetIntValueFromSentence(sentence, key, item);

        if (0 > ret && ParagraphType::NODE == type)
        {
            air::string_view group_name = config_parser.GetStrValueFromSentence(sentence, "Group");
            index = config_parser.GetIndexFromParagraph(ParagraphType::GROUP, group_name);
            sentence = config_parser.GetSentenceFromParagraph(ParagraphType::GROUP, index);
            ret = config_parser.GetIntValueFromSentence(sentence, key);
            if (0 > ret)
            {
                sentence = config_parser.GetSentenceFromParagraph(ParagraphType::DEFAULT, 0);
                ret = config_parser.GetIntValueFromSentence(sentence, key);
            }
        }
        else if (0 > ret && ParagraphType::GROUP == type)
        {
            sentence = config_parser.GetSentenceFromParagraph(ParagraphType::DEFAULT, 0);
            ret = config_parser.GetIntValueFromSentence(sentence, key);
        }

        return ret;
    }

    static constexpr int32_t
    GetIntValue(ParagraphType type, air::string_view key, air::string_view name = "", air::string_view item = "")
    {
        int32_t index = config_parser.GetIndexFromParagraph(type, name);
        return GetIntValue(type, key, index, item);
    }

    static constexpr air::string_view
    GetStrValue(ParagraphType type, air::string_view key, air::string_view name = "")
    {
        int32_t index = config_parser.GetIndexFromParagraph(type, name);
        air::string_view sentence = config_parser.GetSentenceFromParagraph(type, index);
        return config_parser.GetStrValueFromSentence(sentence, key);
    }

    static std::string
    GetItemStrWithNodeName(air::string_view node_name, uint32_t item_index)
    {
        air::string_view filter_name = GetStrValue(ParagraphType::NODE, "Filter", node_name);
        int32_t index = config_parser.GetIndexFromParagraph(ParagraphType::FILTER, filter_name);
        air::string_view sentence = config_parser.GetSentenceFromParagraph(ParagraphType::FILTER, index);
        return config_parser.GetItemStrFromFilterSentence(sentence, item_index);
    }

    static std::string
    GetItemStrWithFilterName(air::string_view filter_name, uint32_t item_index)
    {
        int32_t index = config_parser.GetIndexFromParagraph(ParagraphType::FILTER, filter_name);
        air::string_view sentence = config_parser.GetSentenceFromParagraph(ParagraphType::FILTER, index);
        return config_parser.GetItemStrFromFilterSentence(sentence, item_index);
    }

    static constexpr int32_t
    GetItemSizeWithFilterName(air::string_view filter_name)
    {
        int32_t index = config_parser.GetIndexFromParagraph(ParagraphType::FILTER, filter_name);
        air::string_view sentence = config_parser.GetSentenceFromParagraph(ParagraphType::FILTER, index);
        return config_parser.GetItemSizeFromFilterSentence(sentence);
    }

private:
    static ConfigParser config_parser;
};

} // namespace config

typedef config::ConfigInterface cfg;

#endif // AIR_CONFIG_INTERFACE_H
