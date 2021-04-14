#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/bio/ubio.h"

namespace pos
{
class MockUbio : public Ubio
{
public:
    using Ubio::Ubio;
    MOCK_METHOD(uint32_t, GetOriginCore, (), (override));
};

} // namespace pos
