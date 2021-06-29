
#ifndef AIR_CONFIG_SYNTAX_H
#define AIR_CONFIG_SYNTAX_H

#include "src/config/ConfigChecker.h"
#include "src/config/ConfigLib.h"
#include "src/config/ConfigParser.h"
#include "src/config/ConfigReader.h"
#include "src/lib/StringView.h"

namespace config
{
class ConfigSyntax
{
public:
    constexpr int32_t
    CheckParagraphRule(ParagraphType type)
    {
        return ConfigChecker::CheckParagraphRule(type, paragraphs[dtype(type)]);
    }

    constexpr int32_t
    CheckKeyRule(ParagraphType type)
    {
        const uint32_t paragraph_type_index{dtype(type)};
        int32_t result{0};

        for (uint32_t sentence_index = 0; sentence_index < sentences_count[paragraph_type_index]; sentence_index++)
        {
            result = ConfigChecker::CheckKeyRule(type,
                ConfigParser::GetSentenceFromParagraph(type, sentence_index));
            if (result < 0)
            {
                return result;
            }
        }

        return 0;
    }

    constexpr int32_t
    CheckValueRule(ParagraphType type)
    {
        const uint32_t paragraph_type_index{dtype(type)};
        int32_t result{0};

        for (uint32_t sentence_index = 0; sentence_index < sentences_count[paragraph_type_index]; sentence_index++)
        {
            result = ConfigChecker::CheckValueRule(type,
                ConfigParser::GetSentenceFromParagraph(type, sentence_index));
            if (result < 0)
            {
                return result;
            }
        }

        return 0;
    }
};

class ConfigSyntaxVerification
{
public:
    ConfigSyntaxVerification(void)
    {
        ConfigSyntax syntax;
        static_assert(0 == syntax.CheckParagraphRule(ParagraphType::DEFAULT), "[Default] Paragraph Violation!");
        static_assert(0 == syntax.CheckKeyRule(ParagraphType::DEFAULT), "[Default] Key Violation!");
        static_assert(0 == syntax.CheckValueRule(ParagraphType::DEFAULT), "[Default] Value Violation!");

        static_assert(0 == syntax.CheckParagraphRule(ParagraphType::GROUP), "[Group] Paragraph Violation!");
        static_assert(0 == syntax.CheckKeyRule(ParagraphType::GROUP), "[Group] Key Violation!");
        static_assert(0 == syntax.CheckValueRule(ParagraphType::GROUP), "[Group] Value Violation!");

        static_assert(0 == syntax.CheckParagraphRule(ParagraphType::FILTER), "[Filter] Paragraph Violation!");
        static_assert(0 == syntax.CheckKeyRule(ParagraphType::FILTER), "[Filter] Key Violation!");
        static_assert(0 == syntax.CheckValueRule(ParagraphType::FILTER), "[Filter] Value Violation!");

        static_assert(0 == syntax.CheckParagraphRule(ParagraphType::NODE), "[Node] Paragraph Violation!");
        static_assert(0 == syntax.CheckKeyRule(ParagraphType::NODE), "[Node] Key Violation!");
        static_assert(0 == syntax.CheckValueRule(ParagraphType::NODE), "[Node] Value Violation!");
    }
};

} // namespace config

#endif // AIR_CONFIG_SYNTAX_H
