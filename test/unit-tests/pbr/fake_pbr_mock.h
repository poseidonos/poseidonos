#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/fake_pbr.h"

namespace pbr
{
template <typename T>
class MockFakePbr : public FakePbr<T>
{
public:
    using FakePbr::FakePbr;
};

} // namespace pbr
