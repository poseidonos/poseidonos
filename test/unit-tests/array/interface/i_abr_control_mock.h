#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/interface/i_abr_control.h"

namespace pos
{
class MockIAbrControl : public IAbrControl
{
public:
    using IAbrControl::IAbrControl;
    MOCK_METHOD(int, LoadAbr, (ArrayMeta& meta), (override));
    MOCK_METHOD(int, SaveAbr, (ArrayMeta& meta), (override));
    MOCK_METHOD(int, CreateAbr, (ArrayMeta& meta), (override));
    MOCK_METHOD(int, DeleteAbr, (string name), (override));
    MOCK_METHOD(int, ResetMbr, (), (override));
    MOCK_METHOD(string, FindArrayWithDeviceSN, (string devSN), (override));
    MOCK_METHOD(string, GetLastUpdatedDateTime, (string name), (override));
    MOCK_METHOD(string, GetCreatedDateTime, (string name), (override));
};

} // namespace pos
