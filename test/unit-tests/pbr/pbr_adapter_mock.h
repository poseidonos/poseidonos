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
    MOCK_METHOD(int, Load, (vector<AteData*>& out), (override));
    MOCK_METHOD(int, Reset, (), (override));
    MOCK_METHOD(int, Reset, (string arrayName), (override));
    MOCK_METHOD(int, Update, (AteData* ateData), (override));
};
} // namespace pbr
