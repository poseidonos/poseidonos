#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mbr/abr_manager.h"

namespace pos
{
class MockAbrManager : public AbrManager
{
public:
    using AbrManager::AbrManager;
    MOCK_METHOD(int, LoadAbr, (ArrayMeta& meta), (override));
    MOCK_METHOD(int, SaveAbr, (ArrayMeta& meta), (override));
    MOCK_METHOD(bool, GetMfsInit, (string arrayName), (override));
    MOCK_METHOD(int, SetMfsInit, (string arrayName, bool value), (override));
    MOCK_METHOD(int, CreateAbr, (ArrayMeta& meta), (override));
    MOCK_METHOD(int, DeleteAbr, (ArrayMeta& meta), (override));
    MOCK_METHOD(int, ResetMbr, (), (override));
    MOCK_METHOD(int, GetAbrList, (vector<ArrayBootRecord>& abrList), (override));
    MOCK_METHOD(string, FindArrayWithDeviceSN, (string devSN), (override));
};

} // namespace pos
