#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mbr/mbr_manager.h"

namespace pos
{
class MockMbrManager : public MbrManager
{
public:
    using MbrManager::MbrManager;
    MOCK_METHOD(struct masterBootRecord&, GetMbr, (), (override));
    MOCK_METHOD(int, GetMbrVersionInMemory, (), (override));
    MOCK_METHOD(void, GetAbr, (string targetArrayName, struct ArrayBootRecord** abr, unsigned int& arrayIndex), (override));
    MOCK_METHOD(int, SaveMbr, (), (override));
    MOCK_METHOD(string, FindArrayWithDeviceSN, (string devNSN), (override));
};

} // namespace pos
