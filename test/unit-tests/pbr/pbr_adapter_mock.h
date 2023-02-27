#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/pbr/pbr_adapter.h"

namespace pbr
{
class MockPbrAdapter : public PbrAdapter
{
public:
    using PbrAdapter::PbrAdapter;
    MOCK_METHOD(int, Load, (vector<pos::UblockSharedPtr> devs, vector<AteData*>& out), (override));
    MOCK_METHOD(int, Reset, (vector<pos::UblockSharedPtr> devs), (override));
    MOCK_METHOD(int, Reset, (vector<pos::UblockSharedPtr> devs, string arrayName), (override));
    MOCK_METHOD(int, Update, (vector<pos::UblockSharedPtr> devs, AteData* ateData), (override));
};

} // namespace pbr
