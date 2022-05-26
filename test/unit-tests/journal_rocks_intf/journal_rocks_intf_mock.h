#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_rocks_intf/journal_rocks_intf.h"

namespace pos
{
class MockJournalRocksIntf : public JournalRocksIntf
{
public:
    using JournalRocksIntf::JournalRocksIntf;
    MOCK_METHOD(int, Open, (), (override));
    MOCK_METHOD(bool, Close, (), (override));
    MOCK_METHOD(int, AddJournal, (), (override));
    MOCK_METHOD(int, ReadAllJournal, (), (override));
    MOCK_METHOD(int, ResetJournalByKey, (), (override));
    MOCK_METHOD(int, ResetAllJournal, (), (override));
    MOCK_METHOD(bool, IsOpened, (), (override));
    MOCK_METHOD(bool, DeleteDirectory, (), (override));
    MOCK_METHOD(std::string, GetPathName, (), (override));
    MOCK_METHOD(bool, _CreateDirectory, (), (override));
};

} // namespace pos
