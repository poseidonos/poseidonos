
#ifndef AIR_CONFIG_READER_H
#define AIR_CONFIG_READER_H

#include "src/config/ConfigLib.h"
#include "src/lib/StringView.h"

#define AIR_CFG_FILE <AIR_CFG>

namespace config
{
struct ConfigReader
{
    ConfigReader(void)
    {
    }
    ~ConfigReader(void)
    {
    }
    static constexpr uint32_t
    GetStrArrSize(air::string_view cfg_str)
    {
        size_t count{0};
        size_t pos{0};

        pos = cfg_str.find("\"");
        while (air::string_view::npos != pos)
        {
            pos = cfg_str.find("\"", pos + 1);
            count++;
        }

        return (count / 2);
    }

    static constexpr air::string_view
    GetStrArrFromRawData(air::string_view start_del, air::string_view end_del)
    {
        size_t start_pos = raw_data.find(start_del) + start_del.size() + 1;
        size_t end_pos = raw_data.find(end_del);
        return raw_data.substr(start_pos, end_pos - start_pos);
    }

private:
    static constexpr air::string_view raw_data =
#include AIR_CFG_FILE
        ;
};

static constexpr air::string_view paragraphs[dtype(ParagraphType::COUNT)] = {
    ConfigReader::GetStrArrFromRawData("[DEFAULT]", "[/DEFAULT]"),
    ConfigReader::GetStrArrFromRawData("[GROUP]", "[/GROUP]"),
    ConfigReader::GetStrArrFromRawData("[FILTER]", "[/FILTER]"),
    ConfigReader::GetStrArrFromRawData("[NODE]", "[/NODE]")};

static constexpr uint32_t sentences_count[dtype(ParagraphType::COUNT)] = {
    ConfigReader::GetStrArrSize(paragraphs[dtype(ParagraphType::DEFAULT)]),
    ConfigReader::GetStrArrSize(paragraphs[dtype(ParagraphType::GROUP)]),
    ConfigReader::GetStrArrSize(paragraphs[dtype(ParagraphType::FILTER)]),
    ConfigReader::GetStrArrSize(paragraphs[dtype(ParagraphType::NODE)])};

} // namespace config

#endif // AIR_CONFIG_READER_H
