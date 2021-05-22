
#ifndef AIR_CONFIG_CHECKER_H
#define AIR_CONFIG_CHECKER_H

#include <stdexcept>

#include "src/config/ConfigCheckerKey.h"
#include "src/config/ConfigCheckerLib.h"
#include "src/config/ConfigCheckerValue.h"
#include "src/config/ConfigLib.h"
#include "src/lib/StringView.h"

namespace config
{
class ConfigChecker
{
public:
    ConfigChecker(void)
    {
    }
    virtual ~ConfigChecker(void)
    {
    }

    static constexpr int32_t
    CheckParagraphRule(ParagraphType type, air::string_view paragraph)
    {
        if (0 > CheckParagraphFormat(type, paragraph))
        {
            throw std::logic_error("ParagraphFormat violation");
        }

        if (ParagraphType::DEFAULT == type)
        {
        }
        else if (ParagraphType::GROUP == type)
        {
            air::string_view key = "Group";
            if (0 > CheckValueDuplication(key, paragraph))
            {
                throw std::logic_error("Group value duplicated");
            }
        }
        else if (ParagraphType::FILTER == type)
        {
            air::string_view key = "Filter";
            if (0 > CheckValueDuplication(key, paragraph))
            {
                throw std::logic_error("Filter value duplicated");
            }
        }
        else if (ParagraphType::NODE == type)
        {
            air::string_view key = "Node";
            if (0 > CheckValueDuplication(key, paragraph))
            {
                throw std::logic_error("Node value duplicated");
            }
        }
        else
        {
            throw std::invalid_argument("CheckArrayRule got invalid type");
        }

        return 0;
    }

    static constexpr int32_t
    CheckKeyRule(ParagraphType type, air::string_view sentence)
    {
        uint32_t num_mandatory{num_mandatory_list[dtype(type)]};

        if (ParagraphType::DEFAULT == type)
        {
            if (0 > CheckKeyTypo(type, default_keys, sentence))
            {
                throw std::logic_error("Default key typo");
            }
            if (0 > CheckMandatoryKey(default_keys, num_mandatory, sentence))
            {
                throw std::logic_error("Default mandatory key missing");
            }
        }
        else if (ParagraphType::GROUP == type)
        {
            if (0 > CheckKeyTypo(type, group_keys, sentence))
            {
                throw std::logic_error("Group key typo");
            }
            if (0 > CheckMandatoryKey(group_keys, num_mandatory, sentence))
            {
                throw std::logic_error("Group mandatory key missing");
            }
        }
        else if (ParagraphType::FILTER == type)
        {
            if (0 > CheckKeyTypo(type, filter_keys, sentence))
            {
                throw std::logic_error("Filter key typo");
            }
            if (0 > CheckMandatoryKey(filter_keys, num_mandatory, sentence))
            {
                throw std::logic_error("Filter mandatory key missing");
            }
        }
        else if (ParagraphType::NODE == type)
        {
            if (0 > CheckKeyTypo(type, node_keys, sentence))
            {
                throw std::logic_error("Node key typo");
            }
            if (0 > CheckMandatoryKey(node_keys, num_mandatory, sentence))
            {
                throw std::logic_error("Node mandatory key missing");
            }
        }
        else
        {
            throw std::invalid_argument("CheckKeyRule got invalid type");
        }

        if (0 > CheckKeyDuplication(sentence))
        {
            throw std::logic_error("Key duplicated");
        }

        return 0;
    }

    static constexpr int32_t
    CheckValueRule(ParagraphType type, air::string_view sentence)
    {
        if (0 > ConfigCheckerValue::CheckValueValidity(type, sentence))
        {
            throw std::logic_error("Value violation");
        }

        return 0;
    }

private:
    static constexpr air::string_view default_keys[NUM_DEFAULT_KEY]{
        "StreamingInterval", "AirBuild", "NodeBuild", "NodeRun", "NodeSamplingRatio", "NodeIndexSize"};
    static constexpr air::string_view group_keys[NUM_GROUP_KEY]{
        "Group", "NodeBuild", "NodeRun", "NodeSamplingRatio", "NodeIndexSize"};
    static constexpr air::string_view filter_keys[NUM_FILTER_KEY]{
        "Filter", "Item"};
    static constexpr air::string_view node_keys[NUM_NODE_KEY]{
        "Node", "Type", "Group", "Filter", "Build", "Run", "SamplingRatio", "IndexSize"};

    static constexpr uint32_t num_mandatory_list[4]{6, 1, 2, 4};
};

} // namespace config

#endif // AIR_CONFIG_CHECKER_H
