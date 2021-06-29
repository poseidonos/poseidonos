#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_test.h"

class MockVolumeTest : public VolumeTest
{
public:
    using VolumeTest::VolumeTest;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};
