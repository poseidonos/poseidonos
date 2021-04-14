
#ifndef AIR_JSONRANGE_H
#define AIR_JSONRANGE_H

#include <map>
#include <string>

#include "src/lib/json/Jsondoc.h"

namespace air
{
struct RangeValue
{
    RangeValue(std::string key, JSONtype type, void* value)
    : key{key},
      type{type},
      value{value}
    {
    }
    std::string key;
    JSONtype type;
    void* value;
};

class RangeIterator
{
public:
    explicit RangeIterator(std::map<std::string, JSONvalue>::iterator it)
    : it{it}
    {
    }

    RangeIterator&
    operator++(void)
    {
        ++it;
        return *this;
    }

    bool
    operator!=(const RangeIterator& rhs) const
    {
        return it != rhs.it;
    }

    RangeValue operator*(void)const
    {
        void* vp = (*it).second.data;
        JSONdoc& doc = *((JSONdoc*)vp);
        auto doc_it = doc.map.begin();
        return RangeValue{(*it).first, doc_it->second.type, doc_it->second.data};
    }

private:
    std::map<std::string, JSONvalue>::iterator it;
};

class RangeImpl
{
public:
    explicit RangeImpl(JSONdoc& obj)
    : obj{obj}
    {
    }

    RangeIterator
    begin(void) const
    {
        return RangeIterator{obj.map.begin()};
    }

    RangeIterator
    end(void) const
    {
        return RangeIterator{obj.map.end()};
    }

    static RangeImpl
    range(JSONdoc& obj)
    {
        return RangeImpl{obj};
    }

private:
    JSONdoc& obj;
};

constexpr auto range = RangeImpl::range;

} // namespace air

#endif // AIR_JSONRANGE_H
