#include <gmock/gmock.h>
#include <string>
#include <list>
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
