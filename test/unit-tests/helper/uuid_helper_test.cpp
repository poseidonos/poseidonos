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
    string hyphenRemovedUuid = "";

    // When
    newUuid = UuidHelper::NewUuid();
    hyphenRemovedUuid = UuidHelper::RemoveHyphen(newUuid);

    // Then
    ASSERT_EQ(36, newUuid.length());
    ASSERT_EQ(32, hyphenRemovedUuid.length());
}
