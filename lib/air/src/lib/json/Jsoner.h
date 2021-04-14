
#ifndef AIR_JSONER_H
#define AIR_JSONER_H

#include <map>
#include <stdexcept>
#include <string>

#include "src/lib/json/Jsondoc.h"

namespace air
{
class JSONer
{
public:
    static JSONdoc&
    JSON(std::string key)
    {
        return _GetInstance()._GetDocument(key);
    }
    static void
    Clear(void)
    {
        _GetInstance()._ClearDoc();
    }

private:
    JSONer(void)
    {
        docs.clear();
    }
    ~JSONer(void)
    {
        try
        {
            _ClearDoc();
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    JSONer(const JSONer&) = delete;
    JSONer& operator=(const JSONer&) = delete;

    static JSONer&
    _GetInstance(void)
    {
        static JSONer instance;
        return instance;
    }

    JSONdoc&
    _CreateDocument(std::string key)
    {
        JSONdoc* doc = new JSONdoc{JSONtype::OBJECT, true, false};
        docs.insert({key, doc});
        return *doc;
    }

    JSONdoc&
    _GetDocument(std::string key)
    {
        auto search = docs.find(key);
        if (docs.end() != search)
        {
            return *(search->second);
        }
        else
        {
            return _CreateDocument(key);
        }
    }

    void
    _ClearDoc(void)
    {
        for (const auto& doc : docs)
        {
            doc.second->Clear();
            delete doc.second;
        }
        docs.clear();
    }

    std::map<std::string, JSONdoc*> docs;
};

constexpr auto json = JSONer::JSON;
constexpr auto json_clear = JSONer::Clear;

} // namespace air

#endif // AIR_JSONER_H
