#include <gmock/gmock.h>

#include <string>
#include <list>
#include <vector>

#include "src/journal_manager/checkpoint/dirty_page_list.h"

namespace pos
{
class MockDirtyPageList : public DirtyPageList
{
public:
    using DirtyPageList::DirtyPageList;
    MOCK_METHOD(void, Add, (MapPageList& dirty), (override));
    MOCK_METHOD(MapPageList, GetList, (), (override));
    MOCK_METHOD(void, Reset, (), (override));
    MOCK_METHOD(void, Delete, (int volumeId), (override));
};

} // namespace pos
