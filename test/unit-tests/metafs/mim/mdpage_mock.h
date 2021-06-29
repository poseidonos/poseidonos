#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mdpage.h"

namespace pos
{
class MockMDPage : public MDPage
{
public:
    using MDPage::MDPage;
};

} // namespace pos
