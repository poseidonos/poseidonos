/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "io_submit_handler_test.h"

#include <iostream>
#include <unistd.h>

#include "src/include/memory.h"
#include "src/array/ft/buffer_entry.h"
#include "src/event_scheduler/callback.h"
#include "src/io_scheduler/io_dispatcher.h"

namespace pos
{
class WriteTestCompletion : public Callback
{
public:
    WriteTestCompletion(std::atomic<uint32_t>& testRequests,
        uint32_t inputBufferIndex,
        BufferEntry* inputWriteBufferEntry,
        BufferEntry* inputReadBufferEntry);
    ~WriteTestCompletion(void) override;

private:
    bool _DoSpecificJob(void) override;
    std::atomic<uint32_t>& remainingRequests;
    uint32_t bufferIndex;
    BufferEntry* writeBufferEntry;
    BufferEntry* readBufferEntry;
};

class VerifyTestCompletion : public Callback
{
public:
    VerifyTestCompletion(std::atomic<uint32_t>& testRequests,
        uint32_t inputBufferIndex,
        BufferEntry* inputWriteBufferEntry,
        BufferEntry* inputReadBufferEntry);
    ~VerifyTestCompletion(void) override;

private:
    bool _DoSpecificJob(void) override;
    std::atomic<uint32_t>& remainingRequests;
    uint32_t bufferIndex;
    BufferEntry* writeBufferEntry;
    BufferEntry* readBufferEntry;
};

WriteTestCompletion::WriteTestCompletion(std::atomic<uint32_t>& testRequests,
    uint32_t inputBufferIndex, BufferEntry* inputWriteBufferEntry,
    BufferEntry* inputReadBufferEntry)
: Callback(false),
  remainingRequests(testRequests),
  bufferIndex(inputBufferIndex),
  writeBufferEntry(inputWriteBufferEntry),
  readBufferEntry(inputReadBufferEntry)
{
}

WriteTestCompletion::~WriteTestCompletion(void)
{
}

bool
WriteTestCompletion::_DoSpecificJob(void)
{
    std::cout << "4. AsyncIOTest:: Writing pattern #" << bufferIndex << " done ..." << std::endl;
    // Read and verify
    std::list<BufferEntry> bufferList;
    bufferList.push_back(*readBufferEntry);

    LogicalBlkAddr startLSA = {
        .stripeId = bufferIndex,
        .offset = 0,
    };

    CallbackSmartPtr callback(new VerifyTestCompletion(remainingRequests,
        bufferIndex, writeBufferEntry, readBufferEntry));

    std::cout << "5. AsyncIOTest:: Verifying pattern #" << bufferIndex << " start ..." << std::endl;

    IIOSubmitHandler::GetInstance()->SubmitAsyncIO(IODirection::READ, bufferList,
        startLSA, readBufferEntry->GetBlkCnt(), USER_DATA, callback, 0);

    return true;
}

VerifyTestCompletion::VerifyTestCompletion(
    std::atomic<uint32_t>& testRequests,
    uint32_t inputBufferIndex,
    BufferEntry* inputWriteBufferEntry,
    BufferEntry* inputReadBufferEntry)
: Callback(false),
  remainingRequests(testRequests),
  bufferIndex(inputBufferIndex),
  writeBufferEntry(inputWriteBufferEntry),
  readBufferEntry(inputReadBufferEntry)
{
}

VerifyTestCompletion::~VerifyTestCompletion(void)
{
}

bool
VerifyTestCompletion::_DoSpecificJob(void)
{
    // Read and verify
    bool successful = IOSubmitHandlerTest::VerifyPattern(
        readBufferEntry->GetBufferPtr(),
        writeBufferEntry->GetBufferPtr(),
        readBufferEntry->GetBlkCnt() * BLOCK_SIZE);
    if (false == successful)
    {
        std::cout << "Miscompare!!!!!!" << std::endl;
        std::cout << "Buffer Index: " << bufferIndex << std::endl;
        std::cout << "Block Count: " << readBufferEntry->GetBufferPtr() << std::endl;
    }

    remainingRequests--;

    std::cout << "6. AsyncIOTest:: Verifying pattern #" << bufferIndex << " done ..." << std::endl;

    return true;
}

BufferEntry* IOSubmitHandlerTest::writeBufferEntry[BUFFER_COUNT];
BufferEntry* IOSubmitHandlerTest::readBufferEntry[BUFFER_COUNT];

IOSubmitHandlerTest::IOSubmitHandlerTest(void)
{
}

IOSubmitHandlerTest::~IOSubmitHandlerTest(void)
{
}

bool
IOSubmitHandlerTest::TestSyncIO(void)
{
    std::cout << "Starting SyncIO Test!!!!!" << std::endl;
    bool successful = true;

    std::cout << "1. SyncIOTest:: Allocating Memory..." << std::endl;
    for (uint32_t bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++)
    {
        uint32_t sizeInBlocksWrite = (bufferIndex + 1); // SSD_BLOCKS_PER_CHUNK * SSD_COUNT;
        uint32_t sizeInBlocksRead = (bufferIndex + 1);
        uint32_t sizeInBytesWrite = sizeInBlocksWrite * BLOCK_SIZE;
        uint32_t sizeInBytesRead = sizeInBlocksRead * BLOCK_SIZE;
        writeBufferEntry[bufferIndex] = new BufferEntry(
            malloc(sizeInBytesWrite), sizeInBlocksWrite);
        readBufferEntry[bufferIndex] = new BufferEntry(
            malloc(sizeInBytesRead), sizeInBlocksRead);
    }

    std::cout << "2. SyncIOTest:: Generating Random patterns..." << std::endl;
    for (uint32_t bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++)
    {
        GeneratePattern(writeBufferEntry[bufferIndex]->GetBufferPtr(),
            writeBufferEntry[bufferIndex]->GetBlkCnt() * BLOCK_SIZE);

        std::list<BufferEntry> bufferList;
        bufferList.push_back(*writeBufferEntry[bufferIndex]);

        LogicalBlkAddr startLSA = {
            .stripeId = bufferIndex,
            .offset = 0,
        };

        IIOSubmitHandler::GetInstance()->SyncIO(IODirection::WRITE, bufferList,
            startLSA, writeBufferEntry[bufferIndex]->GetBlkCnt(), USER_DATA, 0);
    }
    std::cout << "3. SyncIOTest:: " << BUFFER_COUNT << " Patterns have been Written ..." << std::endl;

    // Read and verify
    std::cout << "4. SyncIOTest:: Read and Verify All data..." << std::endl;
    for (uint32_t bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++)
    {
        std::list<BufferEntry> bufferList;
        bufferList.push_back(*readBufferEntry[bufferIndex]);

        LogicalBlkAddr startLSA = {
            .stripeId = bufferIndex,
            .offset = 0,
        };

        IIOSubmitHandler::GetInstance()->SyncIO(IODirection::READ, bufferList,
            startLSA, readBufferEntry[bufferIndex]->GetBlkCnt(), USER_DATA, 0);

        successful = VerifyPattern(readBufferEntry[bufferIndex]->GetBufferPtr(),
            writeBufferEntry[bufferIndex]->GetBufferPtr(),
            readBufferEntry[bufferIndex]->GetBlkCnt() * BLOCK_SIZE);

        if (false == successful)
        {
            std::cout << "Miscompare!!!!!!" << std::endl;
            std::cout << "Buffer Index: " << bufferIndex << std::endl;
            std::cout << "Block Count: " << readBufferEntry[bufferIndex]->GetBlkCnt() << std::endl;
            break;
        }
    }
    std::cout << "5. SyncIOTest:: Verify All done..." << std::endl;

    // memory release
    std::cout << "6. SyncIOTest:: Releasing Memory..." << std::endl;
    for (uint32_t bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++)
    {
        free(writeBufferEntry[bufferIndex]->GetBufferPtr());
        free(readBufferEntry[bufferIndex]->GetBufferPtr());
        delete writeBufferEntry[bufferIndex];
        delete readBufferEntry[bufferIndex];
    }

    std::cout << "Ending SyncIO Test!!!!!: Status: " << successful << std::endl;
    return successful;
}

bool
IOSubmitHandlerTest::TestAsyncIO(void)
{
    std::cout << "Starting SubmitAsyncIO Test!!!!!" << std::endl;
    bool successful = true;

    // memory allocation
    std::cout << "1. AsyncIOTest:: Allocating Memory..." << std::endl;
    for (uint32_t bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++)
    {
        uint32_t sizeInBlocksWrite = (bufferIndex + 1); // SSD_BLOCKS_PER_CHUNK * SSD_COUNT;
        uint32_t sizeInBlocksRead = (bufferIndex + 1);
        uint32_t sizeInBytesWrite = sizeInBlocksWrite * BLOCK_SIZE;
        uint32_t sizeInBytesRead = sizeInBlocksRead * BLOCK_SIZE;
        writeBufferEntry[bufferIndex] = new BufferEntry(
            malloc(sizeInBytesWrite), sizeInBlocksWrite);
        readBufferEntry[bufferIndex] = new BufferEntry(
            malloc(sizeInBytesRead), sizeInBlocksRead);
    }

    // generate pattern & write
    std::cout << "2. AsyncIOTest:: Generating Random patterns and Writing..." << std::endl;
    std::atomic<uint32_t> totalRequests(BUFFER_COUNT);

    for (uint32_t bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++)
    {
        GeneratePattern(writeBufferEntry[bufferIndex]->GetBufferPtr(),
            writeBufferEntry[bufferIndex]->GetBlkCnt() * BLOCK_SIZE);

        std::list<BufferEntry> bufferList;
        bufferList.push_back(*writeBufferEntry[bufferIndex]);

        LogicalBlkAddr startLSA = {
            .stripeId = bufferIndex,
            .offset = 0,
        };

        CallbackSmartPtr callback(
            new WriteTestCompletion(totalRequests, bufferIndex,
                writeBufferEntry[bufferIndex], readBufferEntry[bufferIndex]));

        std::cout << "3. AsyncIOTest:: Writing pattern #" << bufferIndex << " start ..." << std::endl;
        IIOSubmitHandler::GetInstance()->SubmitAsyncIO(IODirection::WRITE, bufferList,
            startLSA, writeBufferEntry[bufferIndex]->GetBlkCnt(), USER_DATA,
            callback, 0);
    }

    // wait until all done.
    while (0 < totalRequests)
    {
        usleep(1);
    }
    std::cout << "7. AsyncIOTest:: Verify All done..." << std::endl;

    // memory release
    std::cout << "8. AsyncIOTest:: Releasing Memory..." << std::endl;
    for (uint32_t bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++)
    {
        free(writeBufferEntry[bufferIndex]->GetBufferPtr());
        free(readBufferEntry[bufferIndex]->GetBufferPtr());
        delete writeBufferEntry[bufferIndex];
        delete readBufferEntry[bufferIndex];
    }

    std::cout << "Ending SubmitAsyncIO Test!!!!!: " << successful << std::endl;
    return successful;
}

void
IOSubmitHandlerTest::GeneratePattern(void* mem, unsigned int sizeInBytes)
{
    int* bucket = reinterpret_cast<int*>(mem);
    for (unsigned int index = 0; index < sizeInBytes / sizeof(int); index++)
    {
        bucket[index] = rand_r(&index);
    }
}

bool
IOSubmitHandlerTest::VerifyPattern(
    const void* dst, const void* src, unsigned int sizeInBytes)
{
    int memcmpResult = memcmp(dst, src, sizeInBytes);
    return (0 == memcmpResult);
}

} // namespace pos

int
main(void)
{
    bool successful;

    // Test start
    pos::IOSubmitHandlerTest ioSubmitHandlerTest;

    do
    {
        successful = ioSubmitHandlerTest.TestSyncIO();
        if (false == successful)
        {
            break;
        }

        ioSubmitHandlerTest.TestAsyncIO();
    } while (false);

    return 0;
}
