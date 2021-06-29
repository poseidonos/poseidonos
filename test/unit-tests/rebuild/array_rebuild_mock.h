#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/rebuild/array_rebuild.h"

namespace pos
{
class MockArrayRebuild : public ArrayRebuild
{
public:
    using ArrayRebuild::ArrayRebuild;
};

} // namespace pos
