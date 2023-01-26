#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include "src/helper/uuid/uuid_helper.h"

using ::testing::_;
using ::testing::Return;


TEST(UuidHelper, UuidHelper_GenNewUuidAndVerify)
{
    // Given
    string newUuid = "";

    // When
    newUuid = UuidHelper::GenUuid();
    char uuidByte[16];
    UuidHelper::UuidToByte(newUuid, uuidByte);
    string convertedUuid = UuidHelper::UuidFromByte(uuidByte);

    // Then
    ASSERT_EQ(36, newUuid.length());
    ASSERT_EQ(newUuid, convertedUuid);
}
