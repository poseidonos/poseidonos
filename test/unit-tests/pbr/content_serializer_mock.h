#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/content_serializer.h"

namespace pbr
{
class MockContentSerializer : public ContentSerializer
{
public:
    using ContentSerializer::ContentSerializer;
};

} // namespace pbr
