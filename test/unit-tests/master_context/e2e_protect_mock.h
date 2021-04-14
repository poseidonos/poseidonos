#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/master_context/e2e_protect.h"

namespace pos
{
class MockDataProtect : public DataProtect
{
public:
    using DataProtect::DataProtect;
    MOCK_METHOD(uint32_t, MakeParity, (unsigned char* data, size_t len), (override));
};

} // namespace pos
