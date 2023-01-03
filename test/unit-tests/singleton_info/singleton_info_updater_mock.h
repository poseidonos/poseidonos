#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/singleton_info/singleton_info_updater.h"

namespace pos
{
class MocksingletonInfoUpdater : public SingletonInfo
{
public:
    using SingletonInfo::SingletonInfo;
    MOCK_METHOD(void, Update, (), (override));
};

} // namespace pos
