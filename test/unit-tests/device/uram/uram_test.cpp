#include "src/device/uram/uram.h"

#include <gtest/gtest.h>

#include "test/unit-tests/device/uram/uram_drv_mock.h"

using testing::_;
using testing::NiceMock;
using testing::Return;

using namespace pos;
TEST(Uram, Open_testIfFailedToOpenDeviceDriver)
{
    // Given
    NiceMock<MockUramDrv> mockDrv;
    EXPECT_CALL(mockDrv, Open).WillOnce(Return(false));
    Uram uram("uram", 0, &mockDrv, 0);

    // When
    bool ret = uram.Open();

    // Then
    EXPECT_FALSE(ret);
}

TEST(Uram, Close_testIfUramClosedProperly)
{
    // Given
    NiceMock<MockUramDrv> mockDrv;
    EXPECT_CALL(mockDrv, Open).WillOnce(Return(true));
    EXPECT_CALL(mockDrv, Close).WillOnce(Return(true));
    Uram uram("uram", 0, &mockDrv, 0);
    uram.Open();

    // When
    uint32_t ret = uram.Close();

    // Then
    EXPECT_EQ(ret, 0);
}

TEST(Uram, Close_testIfDeviceContextIsNull)
{
    // Given
    Uram uram("uram", 0, nullptr, 0);

    // When
    uint32_t ret = uram.Close();

    // Then
    EXPECT_EQ(ret, 0);
}

TEST(Uram, GetMN_testIfGetModelNameProperly)
{
    // Given
    string name = "uram";
    Uram uram(name, 0, nullptr, 0);

    // When
    string mn = uram.GetMN();
    // Then
    EXPECT_EQ(mn, name);
}

TEST(Uram, GetClass_testIfGetClassProperly)
{
    // Given
    string name = "uram";
    Uram uram(name, 0, nullptr, 0);

    // When
    DeviceClass cl = uram.GetClass();

    // Then
    EXPECT_EQ(DeviceClass::SYSTEM, cl);
}

TEST(Uram, GetName_testIfGetNameProperly)
{
    // Given
    string name = "uram_get_name_test";
    Uram uram(name, 0, nullptr, 0);

    // When
    string ret = uram.GetName();

    // Then
    EXPECT_EQ(name, ret);
}
