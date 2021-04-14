#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/unvme/unvme_io_context.h"

namespace pos
{
class MockUnvmeIOContext : public UnvmeIOContext
{
public:
    using UnvmeIOContext::UnvmeIOContext;
};

} // namespace pos
