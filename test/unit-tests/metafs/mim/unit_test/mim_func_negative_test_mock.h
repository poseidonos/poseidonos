#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/unit_test/mim_func_negative_test.h"

namespace pos
{
class MockUtMIMFunctionalNegative : public UtMIMFunctionalNegative
{
public:
    using UtMIMFunctionalNegative::UtMIMFunctionalNegative;
};

} // namespace pos
