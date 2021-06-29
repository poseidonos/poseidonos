#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/meta/array_meta.h"

namespace pos
{
class MockArrayMeta : public ArrayMeta
{
public:
    using ArrayMeta::ArrayMeta;
};

} // namespace pos
