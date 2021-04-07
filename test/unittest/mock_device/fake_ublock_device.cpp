#include "src/device/ublock_device.h"

namespace ibofos {

static std::atomic<uint32_t> currentOutstandingIOCount;

UBlockDevice::UBlockDevice(std::string name, uint64_t size,
    DeviceDriver *driverToUse)
:   driver(driverToUse),
    use_(0)
{
}

void
UBlockDevice::RegisterThread(void)
{
}

void
UBlockDevice::UnRegisterThread(void)
{
}

bool
UBlockDevice::OpenDeviceDriver(void)
{
    bool openSuccessful = true;
    return openSuccessful;
}

bool
UBlockDevice::CloseDeviceDriver(void)
{
    bool closeSuccessful = true;
    return closeSuccessful;
}

int
UBlockDevice::AsyncIO(Ubio *bio)
{
    int completions = 0;
    currentOutstandingIOCount++;
    return completions;
}

int
UBlockDevice::BulkAsyncIO(std::vector<Ubio *> &ubioArray, uint32_t ubioCount)
{
    int completions = 0;
    currentOutstandingIOCount += ubioCount;
    return completions;
}

int
UBlockDevice::SyncIO(Ubio *bio)
{
    int completions = 1;
    return completions;
}

int
UBlockDevice::IOGetEvents(void)
{
    int completions = 0;
    if (0 < currentOutstandingIOCount)
    {
        completions = currentOutstandingIOCount;
        currentOutstandingIOCount = 0;
    }

    return completions;
}

int
UBlockDevice::Empty(void)
{
    int completions = 0;
    return completions;
}

}
