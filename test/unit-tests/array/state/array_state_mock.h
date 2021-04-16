#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/state/array_state.h"

namespace pos
{
class MockArrayState : public ArrayState
{
public:
    using ArrayState::ArrayState;
    MOCK_METHOD(void, SetState, (ArrayStateEnum nextState), (override));
    MOCK_METHOD(void, SetLoad, (uint32_t missingCnt, uint32_t brokenCnt), (override));
    MOCK_METHOD(void, SetCreate, (), (override));
    MOCK_METHOD(void, SetDelete, (), (override));
    MOCK_METHOD(bool, SetRebuild, (), (override));
    MOCK_METHOD(void, SetRebuildDone, (bool isSuccess), (override));
    MOCK_METHOD(int, SetMount, (), (override));
    MOCK_METHOD(int, SetUnmount, (), (override));
    MOCK_METHOD(void, SetDegraded, (), (override));
    MOCK_METHOD(int, CanAddSpare, (), (override));
    MOCK_METHOD(int, CanRemoveSpare, (), (override));
    MOCK_METHOD(int, IsLoadable, (), (override));
    MOCK_METHOD(int, IsCreatable, (), (override));
    MOCK_METHOD(int, IsMountable, (), (override));
    MOCK_METHOD(int, IsUnmountable, (), (override));
    MOCK_METHOD(int, IsDeletable, (), (override));
    MOCK_METHOD(bool, IsRebuildable, (), (override));
    MOCK_METHOD(bool, IsRecoverable, (), (override));
    MOCK_METHOD(void, DataRemoved, (bool isRebuildingDevice), (override));
    MOCK_METHOD(bool, Exists, (), (override));
    MOCK_METHOD(bool, IsMounted, (), (override));
    MOCK_METHOD(bool, IsBroken, (), (override));
    MOCK_METHOD(ArrayStateType, GetState, (), (override));
    MOCK_METHOD(StateContext*, GetSysState, (), (override));
    MOCK_METHOD(void, StateChanged, (StateContext * prev, StateContext* next), (override));
};

} // namespace pos
