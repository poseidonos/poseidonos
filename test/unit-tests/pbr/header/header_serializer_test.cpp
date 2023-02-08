#include <gtest/gtest.h>
#include "src/pbr/header/header_serializer.h"
#include "src/include/pos_event_id.h"
 
namespace pbr {

TEST(HeaderSerializer, HeaderSerializer_) {

}

TEST(HeaderSerializer, Serialize_) {

}

TEST(HeaderSerializer, Deserialize_testIfReturnErrorWhenInvalidSignatureIsDetected)
{
    // Given
    IHeaderSerializer* serializer = new HeaderSerializer();
    char rawData[header::LENGTH];
    HeaderElement he;

    // When
    int ret = serializer->Deserialize(rawData, header::LENGTH, &he);

    // Then
    ASSERT_EQ(EID(PBR_UNKNOWN_SIGNATURE), ret);

    // Clean up
    delete serializer;
}

TEST(HeaderSerializer, Deserialize_testIfReturnErrorWhenChecksumIsInvalid)
{
    // Given
    IHeaderSerializer* serializer = new HeaderSerializer();
    char rawData[header::LENGTH];
    HeaderElement he;
    strncpy(&rawData[header::SIGNATURE_OFFSET], header::SIGNATURE.c_str(), header::SIGNATURE_LENGTH);

    // When
    int ret = serializer->Deserialize(rawData, header::LENGTH, &he);

    // Then
    ASSERT_EQ(EID(PBR_CHECKSUM_INVALID), ret);

    // Clean up
    delete serializer;
}

}  // namespace pbr
