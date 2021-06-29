#include <string>

#include "src/io/ubio.h"
#include "src/device/mock_device.h"

static const uint64_t MOCK_DEVICE_SIZE = 2 * 1024 * 1024;
static const std::string MOCK_DEVICE_NAME = "mock";
static const uint64_t BLOCK_SIZE = 4096;

using namespace pos;
using namespace std;
MockDevice*
CreateMockDevice(void)
{
    return new MockDevice(MOCK_DEVICE_NAME.c_str(), MOCK_DEVICE_SIZE);
}

bool
TestOpenAndClose(void)
{
    MockDevice* mockDevice = CreateMockDevice();
    bool success = false;

    success = mockDevice->Open();
    if (success)
    {
        success = mockDevice->Close();
    }

    bool fail = !success;

    delete mockDevice;
    return fail;
}

Ubio*
CreateUbio(UbioDir direction, uint64_t size, uint64_t address, uint64_t pattern)
{
    Ubio* ubio = new Ubio(nullptr, DIV_ROUND_UP(size, Ubio::BYTES_PER_UNIT));
    memset(ubio->GetBuffer(0), pattern, size);
    ubio->dir = direction;
    ubio->address = address;

    return ubio;
}

bool
AsyncIO(MockDevice* mockDevice, Ubio* ubio)
{
    bool fail = true;

    int completion = mockDevice->AsyncIO(ubio);
    if (0 < completion)
    {
        fail = false;
    }

    return fail;
}

bool
Verify(Ubio* ubio, uint64_t size, char pattern)
{
    char* dataBuffer = new char[size];
    memset(dataBuffer, pattern, size);

    bool fail = true;

    if (0 == memcmp(ubio->GetBuffer(0), dataBuffer, size))
    {
        fail = false;
    }

    return fail;
}

bool TestUnmapRead()
{
    MockDevice* mockDevice = CreateMockDevice();
    bool fail = true;

    do
    {
        fail = !(mockDevice->Open());
        if (fail)
        {
            break;
        }

        char pattern = 'A';
        Ubio* ubio = CreateUbio(UbioDir::Readv, BLOCK_SIZE, 0, pattern);

        fail = AsyncIO(mockDevice, ubio);
        if (fail)
        {
            break;
        }

        fail = Verify(ubio, BLOCK_SIZE, 0);
    } while(false);

    delete mockDevice;
    return fail;
}

bool TestWriteAndRead()
{
    MockDevice* mockDevice = CreateMockDevice();
    bool fail = true;

    do
    {
        fail = !(mockDevice->Open());
        if (fail)
        {
            break;
        }

        char writePattern = 'A';
        Ubio* writeUbio = CreateUbio(UbioDir::Writev, BLOCK_SIZE, 0, writePattern);

        fail = AsyncIO(mockDevice, writeUbio);
        if (fail)
        {
            break;
        }

        char readPattern = 'B';
        Ubio* readUbio = CreateUbio(UbioDir::Readv, BLOCK_SIZE, 0, readPattern);

        fail = AsyncIO(mockDevice, readUbio);
        if (fail)
        {
            break;
        }

        fail = Verify(readUbio, BLOCK_SIZE, writePattern);
    } while(false);

    delete mockDevice;
    return fail;
}

bool TestOverWriteAndRead()
{
    MockDevice* mockDevice = CreateMockDevice();
    bool fail = true;

    do
    {
        fail = !(mockDevice->Open());
        if (fail)
        {
            break;
        }

        char writePattern = 'A';
        Ubio* writeUbio = CreateUbio(UbioDir::Writev, BLOCK_SIZE, 0, writePattern);

        fail =  AsyncIO(mockDevice, writeUbio);
        if (fail)
        {
            break;
        }

        char overWritePattern = 'B';
        Ubio* overWriteUbio = CreateUbio(UbioDir::Writev, BLOCK_SIZE, 0, overWritePattern);

        fail = AsyncIO(mockDevice, overWriteUbio);
        if (fail)
        {
            break;
        }

        char readPattern = 'C';
        Ubio* readUbio = CreateUbio(UbioDir::Readv, BLOCK_SIZE, 0, readPattern);

        fail = AsyncIO(mockDevice, readUbio);
        if (fail)
        {
            break;
        }

        fail = Verify(readUbio, BLOCK_SIZE, overWritePattern);
    } while(false);

    delete mockDevice;
    return fail;
}

bool TestWriteAndUnalignedRead()
{
    MockDevice* mockDevice = CreateMockDevice();
    bool fail = true;

    do
    {
        fail = !(mockDevice->Open());
        if (fail)
        {
            break;
        }

        char writePattern = 'A';
        uint64_t writeAddress = 0;
        Ubio* writeUbio = CreateUbio(UbioDir::Writev, BLOCK_SIZE, writeAddress, writePattern);

        fail = AsyncIO(mockDevice, writeUbio);
        if (fail)
        {
            break;
        }

        char readPattern = 'C';
        uint64_t shiftSize = 3;
        uint64_t readAddress = writeAddress + shiftSize;
        uint64_t readSize = BLOCK_SIZE - sector_to_byte(shiftSize);
        Ubio* readUbio = CreateUbio(UbioDir::Readv, readSize, readAddress, readPattern);

        fail = AsyncIO(mockDevice, readUbio);
        if (fail)
        {
            break;
        }

        fail = Verify(readUbio, readSize, writePattern);
    } while(false);

    delete mockDevice;
    return fail;
}
int main(void)
{
    bool fail = false;

    fail = TestOpenAndClose();

    if (fail)
    {
        printf("[MockModuleUT] TestOpenAndClose Fail\n");
    }
    else
    {
        printf("[MockModuleUT] TestOpenAndClose Success\n");
    }
    fail = TestUnmapRead();

    if (fail)
    {
        printf("[MockModuleUT] TestUnmapRead Fail\n");
    }
    else
    {
        printf("[MockModuleUT] TestUnmapRead Success\n");
    }

    fail = TestWriteAndRead();
    if (fail)
    {
        printf("[MockModuleUT] TestWriteAndRead Fail\n");
    }
    else
    {
        printf("[MockModuleUT] TestWriteAndRead Success\n");
    }

    fail = TestOverWriteAndRead();
    if (fail)
    {
        printf("[MockModuleUT] TestOverWriteAndRead Fail\n");
    }
    else
    {
        printf("[MockModuleUT] TestOverWriteAndRead Success\n");
    }

    fail = TestWriteAndUnalignedRead();
    if (fail)
    {
        printf("[MockModuleUT] TestWriteAndUnalignedRead Fail\n");
    }
    else
    {
        printf("[MockModuleUT] TestWriteAndUnalignedRead Success\n");
    }
    return 0;
}
