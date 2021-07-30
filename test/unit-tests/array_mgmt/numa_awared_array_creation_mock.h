#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array_mgmt/numa_awared_array_creation.h"

namespace pos
{
class MockArrayCreationOptions : public ArrayCreationOptions
{
public:
    using ArrayCreationOptions::ArrayCreationOptions;
};

class MockNumaAwaredArrayCreationResult : public NumaAwaredArrayCreationResult
{
public:
    using NumaAwaredArrayCreationResult::NumaAwaredArrayCreationResult;
};

class MockNumaAwaredArrayCreation : public NumaAwaredArrayCreation
{
public:
    using NumaAwaredArrayCreation::NumaAwaredArrayCreation;
};

} // namespace pos
