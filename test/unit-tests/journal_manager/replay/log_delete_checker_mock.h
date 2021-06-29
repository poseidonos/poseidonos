#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/replay/log_delete_checker.h"

namespace pos
{
class MockDeletedVolume : public DeletedVolume
{
public:
    using DeletedVolume::DeletedVolume;
};

class MockLogDeleteChecker : public LogDeleteChecker
{
public:
    using LogDeleteChecker::LogDeleteChecker;
    MOCK_METHOD(std::vector<DeletedVolume>, GetDeletedVolumes, (), (override));
};

} // namespace pos
