#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array_components/array_components.h"

namespace pos
{
class MockArrayComponents : public ArrayComponents
{
public:
    using ArrayComponents::ArrayComponents;
};

} // namespace pos
