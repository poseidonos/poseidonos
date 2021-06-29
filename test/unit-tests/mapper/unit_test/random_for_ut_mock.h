#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/unit_test/random_for_ut.h"

namespace pos
{
class MockRandomForUT : public RandomForUT
{
public:
    using RandomForUT::RandomForUT;
};

} // namespace pos
