#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/checkpoint/dirty_page_list.h"

namespace pos
{
class MockDirtyPageList : public DirtyPageList
{
public:
    using DirtyPageList::DirtyPageList;
};

} // namespace pos
