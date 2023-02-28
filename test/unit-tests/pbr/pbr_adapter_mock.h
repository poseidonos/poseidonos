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
    MOCK_METHOD(int, Load, (const vector<pos::UblockSharedPtr>& devs, vector<unique_ptr<AteData>>& out), (override));
    MOCK_METHOD(int, Reset, (const vector<pos::UblockSharedPtr>& devs), (override));
    MOCK_METHOD(int, Reset, (const vector<pos::UblockSharedPtr>& devs, string arrayName), (override));
    MOCK_METHOD(int, Update, (const vector<pos::UblockSharedPtr>& devs, unique_ptr<AteData> ateData), (override));
};

} // namespace pbr
