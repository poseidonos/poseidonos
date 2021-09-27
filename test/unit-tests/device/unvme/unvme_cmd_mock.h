#include <gmock/gmock.h>

#include "src/device/unvme/unvme_cmd.h"

namespace pos
{
class MockUnvmeCmd : public UnvmeCmd
{
public:
    MOCK_METHOD(int, RequestIO,
        (UnvmeDeviceContext*, spdk_nvme_cmd_cb, UnvmeIOContext*), (override));
};

} // namespace pos
