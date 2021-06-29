#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/unvme/unvme_cmd.h"

namespace pos
{
class MockAbortContext : public AbortContext
{
public:
    using AbortContext::AbortContext;
};

class MockUnvmeCmd : public UnvmeCmd
{
public:
    using UnvmeCmd::UnvmeCmd;
};

} // namespace pos
