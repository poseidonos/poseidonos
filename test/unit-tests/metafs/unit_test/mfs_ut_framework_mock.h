#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/unit_test/mfs_ut_framework.h"

namespace pos
{
class MockMetaFsUtFailTable : public MetaFsUtFailTable
{
public:
    using MetaFsUtFailTable::MetaFsUtFailTable;
};

class MockMetaFsUTException : public MetaFsUTException
{
public:
    using MetaFsUTException::MetaFsUTException;
};

class MockMetaFsUnitTestBase : public MetaFsUnitTestBase
{
public:
    using MetaFsUnitTestBase::MetaFsUnitTestBase;
    MOCK_METHOD(void, Setup, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
    MOCK_METHOD(void, NotifyTCFailAndExit, (), (override));
};

} // namespace pos
