#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/header/header_element.h"

namespace pbr
{
class MockHeaderElement : public HeaderElement
{
public:
    using HeaderElement::HeaderElement;
};

} // namespace pbr
