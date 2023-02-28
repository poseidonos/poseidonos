#include <gtest/gtest.h>
#include "src/pbr/content/content_serializer_factory.h"
 
namespace pbr 
{

TEST(ContentSerializerFactory, GetSerializer_testIfAnInstanceIsReturnedWhenAValidRevisionIsEntered)
{
    // Given
    uint32_t validRevision = 0;

    // When
    auto serializer = pbr::ContentSerializerFactory::GetSerializer(validRevision);

    // Then
    ASSERT_NE(serializer, nullptr);
}

TEST(ContentSerializerFactory, GetSerializer_testIfNullptrIsReturnedWhenAInvalidRevisionIsEntered)
{
    // Given
    uint32_t invalidRevision = 9999;

    // When
    auto serializer = pbr::ContentSerializerFactory::GetSerializer(invalidRevision);

    // Then
    ASSERT_EQ(serializer, nullptr);
}

}  // namespace pbr
