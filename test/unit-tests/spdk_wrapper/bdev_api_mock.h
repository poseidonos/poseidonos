#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/spdk_wrapper/bdev_api.h"

namespace pos
{
class MockBdevApi : public BdevApi
{
public:
    using BdevApi::BdevApi;
};

} // namespace pos
