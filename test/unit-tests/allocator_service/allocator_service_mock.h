#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator_service/allocator_service.h"

namespace pos
{
class MockAllocatorService : public AllocatorService
{
public:
    using AllocatorService::AllocatorService;
    MOCK_METHOD(IBlockAllocator*, GetIBlockAllocator, (std::string arrayName), (override));
    MOCK_METHOD(IWBStripeAllocator*, GetIWBStripeAllocator, (std::string arrayName), (override));
    MOCK_METHOD(IBlockAllocator*, GetIBlockAllocator, (int arrayId), (override));
    MOCK_METHOD(IWBStripeAllocator*, GetIWBStripeAllocator, (int arrayId), (override));
};

} // namespace pos
