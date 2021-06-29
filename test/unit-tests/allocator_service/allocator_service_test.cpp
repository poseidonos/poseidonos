#include "src/allocator_service/allocator_service.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_allocator_wbt_mock.h"

namespace pos
{
TEST(AllocatorService, AllocatorService_)
{
}

TEST(AllocatorService, RegisterAllocator_)
{
}

TEST(AllocatorService, UnregisterAllocator_)
{
}

TEST(AllocatorService, GetIBlockAllocator_)
{
}

TEST(AllocatorService, GetIWBStripeAllocator_)
{
}

TEST(AllocatorService, GetIContextReplayer_TestSimpleGetter)
{
    AllocatorService allocService;
    allocService.GetIContextReplayer("");
}

TEST(AllocatorService, GetIAllocatorWbt_TestSimpleGetter)
{
    AllocatorService allocService;
    allocService.GetIAllocatorWbt("");
}

} // namespace pos
