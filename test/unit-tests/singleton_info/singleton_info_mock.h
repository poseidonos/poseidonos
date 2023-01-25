#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/singleton_info/singleton_info.h"

namespace pos
{
class MocksingletonInfo : public SingletonInfo
{
public:
    using SingletonInfo::SingletonInfo;
    MOCK_METHOD(void, Update, (), (override));
};

} // namespace pos
