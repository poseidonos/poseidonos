#include "src/device/ublock_device.h"

namespace ibofos
{
UBlockDevice::UBlockDevice(std::string name, uint64_t size)
{
  name_ = name;
  size_ = size;
}

UBlockDevice::~UBlockDevice(void)
{

}

bool
UBlockDevice::Open(void)
{
  return true;
}

bool
UBlockDevice::Close(void) 
{
  return true;
}


int
UBlockDevice::BulkAsyncIO(std::vector<Ubio*>& ubioArray, uint32_t ubioCount)
{
    assert(false);
    return 0;
}

}
