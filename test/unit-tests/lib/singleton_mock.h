#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/lib/singleton.h"

namespace pos
{
template<typename T>
class MockSingleton : public Singleton<T>
{
public:
    using Singleton::Singleton;
};

} // namespace pos
