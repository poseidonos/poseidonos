#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/vsamap/i_vsamap_internal.h"

namespace pos
{
class MockIVSAMapInternal : public IVSAMapInternal
{
public:
    using IVSAMapInternal::IVSAMapInternal;
    MOCK_METHOD(int, EnableInternalAccess, (int volID, int caller), (override));
    MOCK_METHOD(std::atomic<int>&, GetLoadDoneFlag, (int volumeId), (override));
};

} // namespace pos
