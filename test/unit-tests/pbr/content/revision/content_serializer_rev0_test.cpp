#include <gtest/gtest.h>
#include "src/pbr/content/revision/content_serializer_rev0.h"
#include "src/include/pos_event_id.h"

namespace pbr
{

TEST(ContentSerializerRev0, Serialize_)
{

}

TEST(ContentSerializerRev0, Deserialize_testIfReturnErrorWhenInvalidSignatureIsDetected)
{
    // Given
    ContentSerializerRev0 serializer;
    char rawData[serializer.GetContentSize()];
    AteData ateData;

    // When
    int ret = serializer.Deserialize(&ateData, rawData);

    // Then
    ASSERT_EQ(EID(ATE_UNKNOWN_SIGNATURE), ret);
}

TEST(ContentSerializerRev0, Deserialize_testIfReturnErrorWhenChecksumIsInvalid)
{
    // Given
    ContentSerializerRev0 serializer;
    char rawData[serializer.GetContentSize()];
    AteData ateData;
    uint32_t backupAteOffset = 24 * 1024;
    uint32_t signatureAbsoluteOffset1 = 0;
    uint32_t signatureAbsoluteOffset2 = backupAteOffset;
    uint32_t signatureLen = 8;
    string signature = "ATE";
    strncpy(&rawData[signatureAbsoluteOffset1], signature.c_str(), signatureLen);
    strncpy(&rawData[signatureAbsoluteOffset2], signature.c_str(), signatureLen);

    // When
    int ret = serializer.Deserialize(&ateData, rawData);

    // Then
    ASSERT_EQ(EID(PBR_CHECKSUM_INVALID), ret);
}

}  // namespace pbr
