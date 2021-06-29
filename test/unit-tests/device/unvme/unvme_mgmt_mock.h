#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/unvme/unvme_mgmt.h"

namespace pos
{
class MockUnvmeMgmt : public UnvmeMgmt
{
public:
    using UnvmeMgmt::UnvmeMgmt;
};

} // namespace pos
