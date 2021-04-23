#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/gc/copier.h"

namespace pos
{
class MockCopier : public Copier
{
public:
    using Copier::Copier;
    MOCK_METHOD(bool, Execute, (), (override));
    MOCK_METHOD(void, Stop, (), (override));
    MOCK_METHOD(bool, IsStopped, (), (override));
    MOCK_METHOD(void, ReadyToEnd, (), (override));
    MOCK_METHOD(void, Pause, (), (override));
    MOCK_METHOD(void, Resume, (), (override));
    MOCK_METHOD(bool, IsPaused, (), (override));
    MOCK_METHOD(void, DisableThresholdCheck, (), (override));
    MOCK_METHOD(bool, IsEnableThresholdCheck, (), (override));
    MOCK_METHOD(CopierStateType, GetCopybackState, (), (override));
};

} // namespace pos
