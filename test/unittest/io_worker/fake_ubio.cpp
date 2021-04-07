#include "src/include/meta_const.h"
#include "src/include/memory.h"
#include "src/io/ubio.h"

namespace ibofos {

Ubio::Ubio(void *buffer, uint32_t unitCount, bool needDeviceLba)
:   dir(UbioDir::Read), sync(false), volumeId(-1), dev(nullptr),
    error(0), vecIdx(0), vecOff(0), size(BYTES_PER_UNIT * unitCount),
    address(0), streamId(0), next(nullptr), endio(nullptr),
    callBackType(EventType_End), syncDone(false),
    remaining(1),
    iov(nullptr), currentIovCount(0), maximumIovCount(0), ibofIo(nullptr)
{
    mem = nullptr;
    memoryOwnership = false;

    if (0 == unitCount)
    {
        return;
    }

    if (nullptr == buffer)
    {
        memoryOwnership = true;
        buffer = ibofos::Memory<BYTES_PER_UNIT>::Alloc(unitCount);
    }

    mem = buffer;
}

Ubio::~Ubio(void)
{
    if (iov)
    {
        free(iov);
    }

    if (memoryOwnership)
    {
        ibofos::Memory<>::Free(mem);
    }
}

void
Ubio::WaitDone(void)
{
    assert(true == sync);

    // Busy polling can be replaced to cond_variable
    // But it is not applied because all sync IO shuold be replaced to async IO
    while(false == syncDone)
    {
        ; // Do nothing
    }
}

int
Ubio::PrepareScatterGatherList(uint32_t sglEntryCountToCreate)
{
    int sglEntryCountLeft = -1;

    if ((nullptr == iov) && (0 < sglEntryCountToCreate)) 
    {
        iov = reinterpret_cast<struct iovec *>(
                    malloc(sglEntryCountToCreate * sizeof(struct iovec *)));
        currentIovCount = 0;
        maximumIovCount = sglEntryCountToCreate;

        if (0 < size)
        {
            iov->iov_base = mem;
            iov->iov_len = size;
            currentIovCount++;
        }

        sglEntryCountLeft = sglEntryCountToCreate - currentIovCount;
    }

    return sglEntryCountLeft;
}

int
Ubio::AddSGLEntry(void *bufferInput, uint32_t sizeInBytes)
{
    int sglEntryCountLeft = -1;

    if (nullptr != iov)
    {
        if (currentIovCount < maximumIovCount)
        {
            iov[currentIovCount].iov_base = bufferInput;
            iov[currentIovCount++].iov_len = sizeInBytes;

            sglEntryCountLeft = maximumIovCount - currentIovCount;
        }
    }

    return sglEntryCountLeft;
}

void *
Ubio::GetBuffer(uint32_t blockIndex, uint32_t sectorOffset)
{
    uint8_t *addr = reinterpret_cast<uint8_t *>(mem);

    if (nullptr != addr)
    {
        addr += (blockIndex * BlockSize) + (sectorOffset * SectorSize);
    }

    return addr;
}

void
Ubio::MarkDone(void)
{
    assert(true == sync);
    assert(false == syncDone);

    syncDone = true;
}

void
Ubio::Complete(void)
{
    if (false != sync)
    {
        MarkDone();
    }
}

}
