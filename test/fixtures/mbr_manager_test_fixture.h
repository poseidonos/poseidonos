#include <gtest/gtest.h>

namespace pos
{
class MbrManagerSuccessfulParameterizedCreateAbrTestFixture : public ::testing::TestWithParam<int>
{
};
class MbrManagerFailedParameterizedCreateAbrTestFixture : public ::testing::TestWithParam<int>
{
};

} // namespace pos