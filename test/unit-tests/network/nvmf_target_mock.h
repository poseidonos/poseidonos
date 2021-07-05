#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/network/nvmf_target.h"

namespace pos
{
class MockNvmfTarget : public NvmfTarget
{
public:
    using NvmfTarget::NvmfTarget;
};

} // namespace pos
