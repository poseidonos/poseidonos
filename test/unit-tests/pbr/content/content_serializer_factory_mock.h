#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/content/content_serializer_factory.h"

namespace pbr
{
class MockContentSerializerFactory : public ContentSerializerFactory
{
public:
    using ContentSerializerFactory::ContentSerializerFactory;
};

} // namespace pbr
