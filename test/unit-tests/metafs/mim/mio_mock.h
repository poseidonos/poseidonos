#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mio.h"

namespace pos
{
class MockMioStateExecuteEntry : public MioStateExecuteEntry
{
public:
    using MioStateExecuteEntry::MioStateExecuteEntry;
};

class MockMio : public Mio
{
public:
    using Mio::Mio;
    MOCK_METHOD(void, InitStateHandler, (), (override));
};

} // namespace pos
