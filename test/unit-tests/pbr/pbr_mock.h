#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/pbr.h"

namespace pbr
{
class MockPbr : public Pbr
{
public:
    using Pbr::Pbr;
    MOCK_METHOD(int, Load, (vector<AteData*>& out, vector<string> fileList), (override));
    MOCK_METHOD(int, Load, (vector<AteData*>& out, vector<pos::UblockSharedPtr> devs), (override));
    MOCK_METHOD(int, Reset, (vector<string> fileList), (override));
    MOCK_METHOD(int, Reset, (vector<pos::UblockSharedPtr> devs), (override));
};

} // namespace pbr
