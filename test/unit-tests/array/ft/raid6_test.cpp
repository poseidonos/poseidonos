#include <gtest/gtest.h>
#include "src/array/ft/raid6.h"
 
#include "src/array_models/dto/partition_physical_size.h"
#include "src/include/array_config.h"
#include "test/unit-tests/resource_manager/buffer_pool_mock.h"
#include "test/unit-tests/resource_manager/memory_manager_mock.h"
#include "test/unit-tests/cpu_affinity/affinity_manager_mock.h"
#include "test/unit-tests/utils/mock_builder.h"

#include <cstring>

namespace pos
{
static BufferEntry
generateRandomBufferEntry(int numBlocks, bool isParity)
{
    int bufSize = ArrayConfig::BLOCK_SIZE_BYTE * numBlocks;
    char* buffer = new char[bufSize];
    unsigned int seed = time(NULL);
    for (int i = 0; i < bufSize; i++)
    {
        buffer[i] = rand_r(&seed);
    }
    return BufferEntry(buffer, numBlocks, isParity);
}

static BufferEntry
generateInitializedBufferEntry(int numBlocks, bool isParity)
{
    int bufSize = ArrayConfig::BLOCK_SIZE_BYTE * numBlocks;
    char* buffer = new char[bufSize];
    memset(buffer, 0, sizeof(buffer));
    return BufferEntry(buffer, numBlocks, isParity);
}

TEST(Raid6, Raid6_testOneDeviceRebuildwithTwoDataDeviceErrorsCase01)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .lastLba = 0 /* not interesting */,
        .blksPerChunk = 64,
        .chunksPerStripe = 14,
        .stripesPerSegment = 0 /* not interesting */,
        .totalSegments = 0 /* not interesting */
    };

    Raid6 raid6(&physicalSize, 0);

    vector<uint32_t> abnormals;
    uint32_t firstErrIdx = 0;
    uint32_t secondErrIdx = 1;

    abnormals.push_back(firstErrIdx);
    abnormals.push_back(secondErrIdx);
    uint32_t nerrs = abnormals.size();

    // When
    uint32_t chunkCnt = physicalSize.chunksPerStripe;
    uint32_t chunkSize = physicalSize.blksPerChunk;
    uint32_t parityCnt = 2;
    uint32_t dataCnt = chunkCnt - parityCnt;
    uint32_t rebuildCnt = chunkCnt - nerrs;

    unsigned char encoding_sources[chunkCnt][chunkSize];
    unsigned char encoding_results_with_errors[chunkSize * rebuildCnt];
    unsigned char decoding_results[chunkSize * nerrs];

    list<BufferEntry> buffers;

    int NUM_BLOCKS = physicalSize.blksPerChunk;

    for (uint32_t i = 0; i < dataCnt; i++)
    {
        buffers.push_back(generateRandomBufferEntry(NUM_BLOCKS, false));
    }

    uint32_t iter = 0;
    unsigned char* src_ptr = nullptr;
    for (const BufferEntry& src_buffer : buffers)
    {
        src_ptr = (unsigned char*)src_buffer.GetBufferPtr();
        memcpy(encoding_sources[iter++], src_ptr, chunkSize);
    }

    uint32_t STRIPEID = 0;
    uint32_t OFFSET = 0;
    LogicalBlkAddr lBlkAddr{
        .stripeId = STRIPEID,
        .offset = OFFSET};

    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = physicalSize.blksPerChunk,
        .buffers = &buffers};

    list<BufferEntry> parities;

    BufferEntry pParity = generateInitializedBufferEntry(NUM_BLOCKS, true);
    BufferEntry qParity = generateInitializedBufferEntry(NUM_BLOCKS, true);

    parities.push_back(pParity);
    parities.push_back(qParity);

    //Then
    raid6.TestParityGeneration(parities, *(src.buffers));
    memcpy(encoding_sources[dataCnt], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);
    memcpy(encoding_sources[dataCnt + 1], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);

    uint32_t tempInx = 0;
    vector<uint32_t> idx_for_test;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            idx_for_test.push_back(tempInx);
        }
        tempInx++;
    }

    uint32_t src_i = 0;
    for (auto idx : idx_for_test)
    {
        for (uint32_t j = 0; j < chunkSize; j++)
        {
            encoding_results_with_errors[src_i * chunkSize + j] = encoding_sources[idx][j];
        }
        src_i++;
    }

    vector<uint32_t> targets;
    targets.push_back(firstErrIdx);
    raid6.TestRebuildData(decoding_results, encoding_results_with_errors, chunkSize, targets, abnormals);
    for (uint32_t i = 0; i < targets.size(); i++)
    {
        ASSERT_EQ(0, memcmp(decoding_results + i * chunkSize, encoding_sources[targets[i]], chunkSize));
    }
}

TEST(Raid6, Raid6_testOneDeviceRebuildwithTwoDataDeviceErrorsCase02)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .lastLba = 0 /* not interesting */,
        .blksPerChunk = 64,
        .chunksPerStripe = 14,
        .stripesPerSegment = 0 /* not interesting */,
        .totalSegments = 0 /* not interesting */
    };

    Raid6 raid6(&physicalSize, 0);

    vector<uint32_t> abnormals;
    uint32_t firstErrIdx = 4;
    uint32_t secondErrIdx = 8;

    abnormals.push_back(firstErrIdx);
    abnormals.push_back(secondErrIdx);
    uint32_t nerrs = abnormals.size();

    // When
    uint32_t chunkCnt = physicalSize.chunksPerStripe;
    uint32_t chunkSize = physicalSize.blksPerChunk;
    uint32_t parityCnt = 2;
    uint32_t dataCnt = chunkCnt - parityCnt;
    uint32_t rebuildCnt = chunkCnt - nerrs;

    unsigned char encoding_sources[chunkCnt][chunkSize];
    unsigned char encoding_results_with_errors[chunkSize * rebuildCnt];
    unsigned char decoding_results[chunkSize * nerrs];

    list<BufferEntry> buffers;

    int NUM_BLOCKS = physicalSize.blksPerChunk;

    for (uint32_t i = 0; i < dataCnt; i++)
    {
        buffers.push_back(generateRandomBufferEntry(NUM_BLOCKS, false));
    }

    uint32_t iter = 0;
    unsigned char* src_ptr = nullptr;
    for (const BufferEntry& src_buffer : buffers)
    {
        src_ptr = (unsigned char*)src_buffer.GetBufferPtr();
        memcpy(encoding_sources[iter++], src_ptr, chunkSize);
    }

    uint32_t STRIPEID = 0;
    uint32_t OFFSET = 0;
    LogicalBlkAddr lBlkAddr{
        .stripeId = STRIPEID,
        .offset = OFFSET};

    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = physicalSize.blksPerChunk,
        .buffers = &buffers};

    list<BufferEntry> parities;

    BufferEntry pParity = generateInitializedBufferEntry(NUM_BLOCKS, true);
    BufferEntry qParity = generateInitializedBufferEntry(NUM_BLOCKS, true);

    parities.push_back(pParity);
    parities.push_back(qParity);

    //Then
    raid6.TestParityGeneration(parities, *(src.buffers));
    memcpy(encoding_sources[dataCnt], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);
    memcpy(encoding_sources[dataCnt + 1], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);

    uint32_t tempInx = 0;
    vector<uint32_t> idx_for_test;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            idx_for_test.push_back(tempInx);
        }
        tempInx++;
    }

    uint32_t src_i = 0;
    for (auto idx : idx_for_test)
    {
        for (uint32_t j = 0; j < chunkSize; j++)
        {
            encoding_results_with_errors[src_i * chunkSize + j] = encoding_sources[idx][j];
        }
        src_i++;
    }

    vector<uint32_t> targets;
    targets.push_back(firstErrIdx);
    raid6.TestRebuildData(decoding_results, encoding_results_with_errors, chunkSize, targets, abnormals);
    for (uint32_t i = 0; i < targets.size(); i++)
    {
        ASSERT_EQ(0, memcmp(decoding_results + i * chunkSize, encoding_sources[targets[i]], chunkSize));
    }
}

TEST(Raid6, Raid6_testOneDeviceRebuildwithTwoDataDeviceErrorsCase03)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .lastLba = 0 /* not interesting */,
        .blksPerChunk = 64,
        .chunksPerStripe = 14,
        .stripesPerSegment = 0 /* not interesting */,
        .totalSegments = 0 /* not interesting */
    };

    Raid6 raid6(&physicalSize, 0);

    vector<uint32_t> abnormals;
    uint32_t firstErrIdx = 13;
    uint32_t secondErrIdx = 14;

    abnormals.push_back(firstErrIdx);
    abnormals.push_back(secondErrIdx);
    uint32_t nerrs = abnormals.size();

    // When
    uint32_t chunkCnt = physicalSize.chunksPerStripe;
    uint32_t chunkSize = physicalSize.blksPerChunk;
    uint32_t parityCnt = 2;
    uint32_t dataCnt = chunkCnt - parityCnt;
    uint32_t rebuildCnt = chunkCnt - nerrs;

    unsigned char encoding_sources[chunkCnt][chunkSize];
    unsigned char encoding_results_with_errors[chunkSize * rebuildCnt];
    unsigned char decoding_results[chunkSize * nerrs];

    list<BufferEntry> buffers;

    int NUM_BLOCKS = physicalSize.blksPerChunk;

    for (uint32_t i = 0; i < dataCnt; i++)
    {
        buffers.push_back(generateRandomBufferEntry(NUM_BLOCKS, false));
    }

    uint32_t iter = 0;
    unsigned char* src_ptr = nullptr;
    for (const BufferEntry& src_buffer : buffers)
    {
        src_ptr = (unsigned char*)src_buffer.GetBufferPtr();
        memcpy(encoding_sources[iter++], src_ptr, chunkSize);
    }

    uint32_t STRIPEID = 0;
    uint32_t OFFSET = 0;
    LogicalBlkAddr lBlkAddr{
        .stripeId = STRIPEID,
        .offset = OFFSET};

    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = physicalSize.blksPerChunk,
        .buffers = &buffers};

    list<BufferEntry> parities;

    BufferEntry pParity = generateInitializedBufferEntry(NUM_BLOCKS, true);
    BufferEntry qParity = generateInitializedBufferEntry(NUM_BLOCKS, true);

    parities.push_back(pParity);
    parities.push_back(qParity);

    //Then
    raid6.TestParityGeneration(parities, *(src.buffers));
    memcpy(encoding_sources[dataCnt], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);
    memcpy(encoding_sources[dataCnt + 1], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);

    uint32_t tempInx = 0;
    vector<uint32_t> idx_for_test;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            idx_for_test.push_back(tempInx);
        }
        tempInx++;
    }

    uint32_t src_i = 0;
    for (auto idx : idx_for_test)
    {
        for (uint32_t j = 0; j < chunkSize; j++)
        {
            encoding_results_with_errors[src_i * chunkSize + j] = encoding_sources[idx][j];
        }
        src_i++;
    }

    vector<uint32_t> targets;
    targets.push_back(firstErrIdx);
    raid6.TestRebuildData(decoding_results, encoding_results_with_errors, chunkSize, targets, abnormals);
    for (uint32_t i = 0; i < targets.size(); i++)
    {
        ASSERT_EQ(0, memcmp(decoding_results + i * chunkSize, encoding_sources[targets[i]], chunkSize));
    }
}

TEST(Raid6, Raid6_testOneDeviceRebuildwithOneDataDeviceErrorsCase01)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .lastLba = 0 /* not interesting */,
        .blksPerChunk = 64,
        .chunksPerStripe = 14,
        .stripesPerSegment = 0 /* not interesting */,
        .totalSegments = 0 /* not interesting */
    };

    Raid6 raid6(&physicalSize, 0);

    vector<uint32_t> abnormals;
    uint32_t firstErrIdx = 0;

    abnormals.push_back(firstErrIdx);
    uint32_t nerrs = abnormals.size();

    // When
    uint32_t chunkCnt = physicalSize.chunksPerStripe;
    uint32_t chunkSize = physicalSize.blksPerChunk;
    uint32_t parityCnt = 2;
    uint32_t dataCnt = chunkCnt - parityCnt;
    uint32_t rebuildCnt = chunkCnt - nerrs;

    unsigned char encoding_sources[chunkCnt][chunkSize];
    unsigned char encoding_results_with_errors[chunkSize * rebuildCnt];
    unsigned char decoding_results[chunkSize * nerrs];

    list<BufferEntry> buffers;

    int NUM_BLOCKS = physicalSize.blksPerChunk;

    for (uint32_t i = 0; i < dataCnt; i++)
    {
        buffers.push_back(generateRandomBufferEntry(NUM_BLOCKS, false));
    }

    uint32_t iter = 0;
    unsigned char* src_ptr = nullptr;
    for (const BufferEntry& src_buffer : buffers)
    {
        src_ptr = (unsigned char*)src_buffer.GetBufferPtr();
        memcpy(encoding_sources[iter++], src_ptr, chunkSize);
    }

    uint32_t STRIPEID = 0;
    uint32_t OFFSET = 0;
    LogicalBlkAddr lBlkAddr{
        .stripeId = STRIPEID,
        .offset = OFFSET};

    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = physicalSize.blksPerChunk,
        .buffers = &buffers};

    list<BufferEntry> parities;

    BufferEntry pParity = generateInitializedBufferEntry(NUM_BLOCKS, true);
    BufferEntry qParity = generateInitializedBufferEntry(NUM_BLOCKS, true);

    parities.push_back(pParity);
    parities.push_back(qParity);

    //Then
    raid6.TestParityGeneration(parities, *(src.buffers));
    memcpy(encoding_sources[dataCnt], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);
    memcpy(encoding_sources[dataCnt + 1], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);

    uint32_t tempInx = 0;
    vector<uint32_t> idx_for_test;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            idx_for_test.push_back(tempInx);
        }
        tempInx++;
    }

    uint32_t src_i = 0;
    for (auto idx : idx_for_test)
    {
        for (uint32_t j = 0; j < chunkSize; j++)
        {
            encoding_results_with_errors[src_i * chunkSize + j] = encoding_sources[idx][j];
        }
        src_i++;
    }

    vector<uint32_t> targets;
    targets.push_back(firstErrIdx);
    raid6.TestRebuildData(decoding_results, encoding_results_with_errors, chunkSize, targets, abnormals);
    for (uint32_t i = 0; i < targets.size(); i++)
    {
        ASSERT_EQ(0, memcmp(decoding_results + i * chunkSize, encoding_sources[targets[i]], chunkSize));
    }
}

TEST(Raid6, Raid6_testOneDeviceRebuildwithOneDataDeviceErrorsCase02)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .lastLba = 0 /* not interesting */,
        .blksPerChunk = 64,
        .chunksPerStripe = 14,
        .stripesPerSegment = 0 /* not interesting */,
        .totalSegments = 0 /* not interesting */
    };

    Raid6 raid6(&physicalSize, 0);

    vector<uint32_t> abnormals;
    uint32_t firstErrIdx = 10;

    abnormals.push_back(firstErrIdx);
    uint32_t nerrs = abnormals.size();

    // When
    uint32_t chunkCnt = physicalSize.chunksPerStripe;
    uint32_t chunkSize = physicalSize.blksPerChunk;
    uint32_t parityCnt = 2;
    uint32_t dataCnt = chunkCnt - parityCnt;
    uint32_t rebuildCnt = chunkCnt - nerrs;

    unsigned char encoding_sources[chunkCnt][chunkSize];
    unsigned char encoding_results_with_errors[chunkSize * rebuildCnt];
    unsigned char decoding_results[chunkSize * nerrs];

    list<BufferEntry> buffers;

    int NUM_BLOCKS = physicalSize.blksPerChunk;

    for (uint32_t i = 0; i < dataCnt; i++)
    {
        buffers.push_back(generateRandomBufferEntry(NUM_BLOCKS, false));
    }

    uint32_t iter = 0;
    unsigned char* src_ptr = nullptr;
    for (const BufferEntry& src_buffer : buffers)
    {
        src_ptr = (unsigned char*)src_buffer.GetBufferPtr();
        memcpy(encoding_sources[iter++], src_ptr, chunkSize);
    }

    uint32_t STRIPEID = 0;
    uint32_t OFFSET = 0;
    LogicalBlkAddr lBlkAddr{
        .stripeId = STRIPEID,
        .offset = OFFSET};

    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = physicalSize.blksPerChunk,
        .buffers = &buffers};

    list<BufferEntry> parities;

    BufferEntry pParity = generateInitializedBufferEntry(NUM_BLOCKS, true);
    BufferEntry qParity = generateInitializedBufferEntry(NUM_BLOCKS, true);

    parities.push_back(pParity);
    parities.push_back(qParity);

    //Then
    raid6.TestParityGeneration(parities, *(src.buffers));
    memcpy(encoding_sources[dataCnt], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);
    memcpy(encoding_sources[dataCnt + 1], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);

    uint32_t tempInx = 0;
    vector<uint32_t> idx_for_test;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            idx_for_test.push_back(tempInx);
        }
        tempInx++;
    }

    uint32_t src_i = 0;
    for (auto idx : idx_for_test)
    {
        for (uint32_t j = 0; j < chunkSize; j++)
        {
            encoding_results_with_errors[src_i * chunkSize + j] = encoding_sources[idx][j];
        }
        src_i++;
    }

    vector<uint32_t> targets;
    targets.push_back(firstErrIdx);
    raid6.TestRebuildData(decoding_results, encoding_results_with_errors, chunkSize, targets, abnormals);
    for (uint32_t i = 0; i < targets.size(); i++)
    {
        ASSERT_EQ(0, memcmp(decoding_results + i * chunkSize, encoding_sources[targets[i]], chunkSize));
    }
}

TEST(Raid6, Raid6_testOneDeviceRebuildwithTwoParityDeviceErrorsCase01)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .lastLba = 0 /* not interesting */,
        .blksPerChunk = 64,
        .chunksPerStripe = 14,
        .stripesPerSegment = 0 /* not interesting */,
        .totalSegments = 0 /* not interesting */
    };

    Raid6 raid6(&physicalSize, 0);

    vector<uint32_t> abnormals;
    uint32_t firstErrIdx = physicalSize.chunksPerStripe - 2;
    uint32_t secondErrIdx = physicalSize.chunksPerStripe - 1;

    abnormals.push_back(firstErrIdx);
    abnormals.push_back(secondErrIdx);
    uint32_t nerrs = abnormals.size();

    // When
    uint32_t chunkCnt = physicalSize.chunksPerStripe;
    uint32_t chunkSize = physicalSize.blksPerChunk;
    uint32_t parityCnt = 2;
    uint32_t dataCnt = chunkCnt - parityCnt;
    uint32_t rebuildCnt = chunkCnt - nerrs;

    unsigned char encoding_sources[chunkCnt][chunkSize];
    unsigned char encoding_results_with_errors[chunkSize * rebuildCnt];
    unsigned char decoding_results[chunkSize * nerrs];

    list<BufferEntry> buffers;

    int NUM_BLOCKS = physicalSize.blksPerChunk;

    for (uint32_t i = 0; i < dataCnt; i++)
    {
        buffers.push_back(generateRandomBufferEntry(NUM_BLOCKS, false));
    }

    uint32_t iter = 0;
    unsigned char* src_ptr = nullptr;
    for (const BufferEntry& src_buffer : buffers)
    {
        src_ptr = (unsigned char*)src_buffer.GetBufferPtr();
        memcpy(encoding_sources[iter++], src_ptr, chunkSize);
    }

    uint32_t STRIPEID = 0;
    uint32_t OFFSET = 0;
    LogicalBlkAddr lBlkAddr{
        .stripeId = STRIPEID,
        .offset = OFFSET};

    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = physicalSize.blksPerChunk,
        .buffers = &buffers};

    list<BufferEntry> parities;

    BufferEntry pParity = generateInitializedBufferEntry(NUM_BLOCKS, true);
    BufferEntry qParity = generateInitializedBufferEntry(NUM_BLOCKS, true);

    parities.push_back(pParity);
    parities.push_back(qParity);

    //Then
    raid6.TestParityGeneration(parities, *(src.buffers));
    memcpy(encoding_sources[dataCnt], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);
    memcpy(encoding_sources[dataCnt + 1], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);

    uint32_t tempInx = 0;
    vector<uint32_t> idx_for_test;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            idx_for_test.push_back(tempInx);
        }
        tempInx++;
    }

    uint32_t src_i = 0;
    for (auto idx : idx_for_test)
    {
        for (uint32_t j = 0; j < chunkSize; j++)
        {
            encoding_results_with_errors[src_i * chunkSize + j] = encoding_sources[idx][j];
        }
        src_i++;
    }

    vector<uint32_t> targets;
    targets.push_back(firstErrIdx);
    raid6.TestRebuildData(decoding_results, encoding_results_with_errors, chunkSize, targets, abnormals);
    for (uint32_t i = 0; i < targets.size(); i++)
    {
        ASSERT_EQ(0, memcmp(decoding_results + i * chunkSize, encoding_sources[targets[i]], chunkSize));
    }
}

TEST(Raid6, Raid6_testOneDeviceRebuildwithOneParityDeviceErrorsCase01)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .lastLba = 0 /* not interesting */,
        .blksPerChunk = 64,
        .chunksPerStripe = 14,
        .stripesPerSegment = 0 /* not interesting */,
        .totalSegments = 0 /* not interesting */
    };

    Raid6 raid6(&physicalSize, 0);

    vector<uint32_t> abnormals;
    uint32_t firstErrIdx = physicalSize.chunksPerStripe - 2;

    abnormals.push_back(firstErrIdx);
    uint32_t nerrs = abnormals.size();

    // When
    uint32_t chunkCnt = physicalSize.chunksPerStripe;
    uint32_t chunkSize = physicalSize.blksPerChunk;
    uint32_t parityCnt = 2;
    uint32_t dataCnt = chunkCnt - parityCnt;
    uint32_t rebuildCnt = chunkCnt - nerrs;

    unsigned char encoding_sources[chunkCnt][chunkSize];
    unsigned char encoding_results_with_errors[chunkSize * rebuildCnt];
    unsigned char decoding_results[chunkSize * nerrs];

    list<BufferEntry> buffers;

    int NUM_BLOCKS = physicalSize.blksPerChunk;

    for (uint32_t i = 0; i < dataCnt; i++)
    {
        buffers.push_back(generateRandomBufferEntry(NUM_BLOCKS, false));
    }

    uint32_t iter = 0;
    unsigned char* src_ptr = nullptr;
    for (const BufferEntry& src_buffer : buffers)
    {
        src_ptr = (unsigned char*)src_buffer.GetBufferPtr();
        memcpy(encoding_sources[iter++], src_ptr, chunkSize);
    }

    uint32_t STRIPEID = 0;
    uint32_t OFFSET = 0;
    LogicalBlkAddr lBlkAddr{
        .stripeId = STRIPEID,
        .offset = OFFSET};

    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = physicalSize.blksPerChunk,
        .buffers = &buffers};

    list<BufferEntry> parities;

    BufferEntry pParity = generateInitializedBufferEntry(NUM_BLOCKS, true);
    BufferEntry qParity = generateInitializedBufferEntry(NUM_BLOCKS, true);

    parities.push_back(pParity);
    parities.push_back(qParity);

    //Then
    raid6.TestParityGeneration(parities, *(src.buffers));
    memcpy(encoding_sources[dataCnt], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);
    memcpy(encoding_sources[dataCnt + 1], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);

    uint32_t tempInx = 0;
    vector<uint32_t> idx_for_test;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            idx_for_test.push_back(tempInx);
        }
        tempInx++;
    }

    uint32_t src_i = 0;
    for (auto idx : idx_for_test)
    {
        for (uint32_t j = 0; j < chunkSize; j++)
        {
            encoding_results_with_errors[src_i * chunkSize + j] = encoding_sources[idx][j];
        }
        src_i++;
    }

    vector<uint32_t> targets;
    targets.push_back(firstErrIdx);
    raid6.TestRebuildData(decoding_results, encoding_results_with_errors, chunkSize, targets, abnormals);
    for (uint32_t i = 0; i < targets.size(); i++)
    {
        ASSERT_EQ(0, memcmp(decoding_results + i * chunkSize, encoding_sources[targets[i]], chunkSize));
    }
}

TEST(Raid6, Raid6_testOneDeviceRebuildwithOneParityDeviceErrorsCase02)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .lastLba = 0 /* not interesting */,
        .blksPerChunk = 64,
        .chunksPerStripe = 14,
        .stripesPerSegment = 0 /* not interesting */,
        .totalSegments = 0 /* not interesting */
    };

    Raid6 raid6(&physicalSize, 0);

    vector<uint32_t> abnormals;
    uint32_t firstErrIdx = physicalSize.chunksPerStripe - 1;

    abnormals.push_back(firstErrIdx);
    uint32_t nerrs = abnormals.size();

    // When
    uint32_t chunkCnt = physicalSize.chunksPerStripe;
    uint32_t chunkSize = physicalSize.blksPerChunk;
    uint32_t parityCnt = 2;
    uint32_t dataCnt = chunkCnt - parityCnt;
    uint32_t rebuildCnt = chunkCnt - nerrs;

    unsigned char encoding_sources[chunkCnt][chunkSize];
    unsigned char encoding_results_with_errors[chunkSize * rebuildCnt];
    unsigned char decoding_results[chunkSize * nerrs];

    list<BufferEntry> buffers;

    int NUM_BLOCKS = physicalSize.blksPerChunk;

    for (uint32_t i = 0; i < dataCnt; i++)
    {
        buffers.push_back(generateRandomBufferEntry(NUM_BLOCKS, false));
    }

    uint32_t iter = 0;
    unsigned char* src_ptr = nullptr;
    for (const BufferEntry& src_buffer : buffers)
    {
        src_ptr = (unsigned char*)src_buffer.GetBufferPtr();
        memcpy(encoding_sources[iter++], src_ptr, chunkSize);
    }

    uint32_t STRIPEID = 0;
    uint32_t OFFSET = 0;
    LogicalBlkAddr lBlkAddr{
        .stripeId = STRIPEID,
        .offset = OFFSET};

    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = physicalSize.blksPerChunk,
        .buffers = &buffers};

    list<BufferEntry> parities;

    BufferEntry pParity = generateInitializedBufferEntry(NUM_BLOCKS, true);
    BufferEntry qParity = generateInitializedBufferEntry(NUM_BLOCKS, true);

    parities.push_back(pParity);
    parities.push_back(qParity);

    //Then
    raid6.TestParityGeneration(parities, *(src.buffers));
    memcpy(encoding_sources[dataCnt], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);
    memcpy(encoding_sources[dataCnt + 1], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);

    uint32_t tempInx = 0;
    vector<uint32_t> idx_for_test;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            idx_for_test.push_back(tempInx);
        }
        tempInx++;
    }

    uint32_t src_i = 0;
    for (auto idx : idx_for_test)
    {
        for (uint32_t j = 0; j < chunkSize; j++)
        {
            encoding_results_with_errors[src_i * chunkSize + j] = encoding_sources[idx][j];
        }
        src_i++;
    }

    vector<uint32_t> targets;
    targets.push_back(firstErrIdx);
    raid6.TestRebuildData(decoding_results, encoding_results_with_errors, chunkSize, targets, abnormals);
    for (uint32_t i = 0; i < targets.size(); i++)
    {
        ASSERT_EQ(0, memcmp(decoding_results + i * chunkSize, encoding_sources[targets[i]], chunkSize));
    }
}

TEST(Raid6, Raid6_testOneDeviceRebuildwithTwoParityDeviceErrorsCase02)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .lastLba = 0 /* not interesting */,
        .blksPerChunk = 64,
        .chunksPerStripe = 14,
        .stripesPerSegment = 0 /* not interesting */,
        .totalSegments = 0 /* not interesting */
    };

    Raid6 raid6(&physicalSize, 0);

    vector<uint32_t> abnormals;
    uint32_t firstErrIdx = physicalSize.chunksPerStripe - 1;
    uint32_t secondErrIdx = physicalSize.chunksPerStripe - 2;

    abnormals.push_back(firstErrIdx);
    abnormals.push_back(secondErrIdx);
    uint32_t nerrs = abnormals.size();

    // When
    uint32_t chunkCnt = physicalSize.chunksPerStripe;
    uint32_t chunkSize = physicalSize.blksPerChunk;
    uint32_t parityCnt = 2;
    uint32_t dataCnt = chunkCnt - parityCnt;
    uint32_t rebuildCnt = chunkCnt - nerrs;

    unsigned char encoding_sources[chunkCnt][chunkSize];
    unsigned char encoding_results_with_errors[chunkSize * rebuildCnt];
    unsigned char decoding_results[chunkSize * nerrs];

    list<BufferEntry> buffers;

    int NUM_BLOCKS = physicalSize.blksPerChunk;

    for (uint32_t i = 0; i < dataCnt; i++)
    {
        buffers.push_back(generateRandomBufferEntry(NUM_BLOCKS, false));
    }

    uint32_t iter = 0;
    unsigned char* src_ptr = nullptr;
    for (const BufferEntry& src_buffer : buffers)
    {
        src_ptr = (unsigned char*)src_buffer.GetBufferPtr();
        memcpy(encoding_sources[iter++], src_ptr, chunkSize);
    }

    uint32_t STRIPEID = 0;
    uint32_t OFFSET = 0;
    LogicalBlkAddr lBlkAddr{
        .stripeId = STRIPEID,
        .offset = OFFSET};

    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = physicalSize.blksPerChunk,
        .buffers = &buffers};

    list<BufferEntry> parities;

    BufferEntry pParity = generateInitializedBufferEntry(NUM_BLOCKS, true);
    BufferEntry qParity = generateInitializedBufferEntry(NUM_BLOCKS, true);

    parities.push_back(pParity);
    parities.push_back(qParity);

    //Then
    raid6.TestParityGeneration(parities, *(src.buffers));
    memcpy(encoding_sources[dataCnt], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);
    memcpy(encoding_sources[dataCnt + 1], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);

    uint32_t tempInx = 0;
    vector<uint32_t> idx_for_test;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            idx_for_test.push_back(tempInx);
        }
        tempInx++;
    }

    uint32_t src_i = 0;
    for (auto idx : idx_for_test)
    {
        for (uint32_t j = 0; j < chunkSize; j++)
        {
            encoding_results_with_errors[src_i * chunkSize + j] = encoding_sources[idx][j];
        }
        src_i++;
    }

    vector<uint32_t> targets;
    targets.push_back(firstErrIdx);
    raid6.TestRebuildData(decoding_results, encoding_results_with_errors, chunkSize, targets, abnormals);
    for (uint32_t i = 0; i < targets.size(); i++)
    {
        ASSERT_EQ(0, memcmp(decoding_results + i * chunkSize, encoding_sources[targets[i]], chunkSize));
    }
}

TEST(Raid6, Raid6_testTwoDeviceRebuildwithTwoDataDeviceErrorsCase01)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .lastLba = 0 /* not interesting */,
        .blksPerChunk = 64,
        .chunksPerStripe = 14,
        .stripesPerSegment = 0 /* not interesting */,
        .totalSegments = 0 /* not interesting */
    };

    Raid6 raid6(&physicalSize, 0);

    vector<uint32_t> abnormals;
    uint32_t firstErrIdx = 0;
    uint32_t secondErrIdx = 2;

    abnormals.push_back(firstErrIdx);
    abnormals.push_back(secondErrIdx);
    uint32_t nerrs = abnormals.size();

    // When
    uint32_t chunkCnt = physicalSize.chunksPerStripe;
    uint32_t chunkSize = physicalSize.blksPerChunk;
    uint32_t parityCnt = 2;
    uint32_t dataCnt = chunkCnt - parityCnt;
    uint32_t rebuildCnt = chunkCnt - nerrs;

    unsigned char encoding_sources[chunkCnt][chunkSize];
    unsigned char encoding_results_with_errors[chunkSize * rebuildCnt];
    unsigned char decoding_results[chunkSize * nerrs];

    list<BufferEntry> buffers;

    int NUM_BLOCKS = physicalSize.blksPerChunk;

    for (uint32_t i = 0; i < dataCnt; i++)
    {
        buffers.push_back(generateRandomBufferEntry(NUM_BLOCKS, false));
    }

    uint32_t iter = 0;
    unsigned char* src_ptr = nullptr;
    for (const BufferEntry& src_buffer : buffers)
    {
        src_ptr = (unsigned char*)src_buffer.GetBufferPtr();
        memcpy(encoding_sources[iter++], src_ptr, chunkSize);
    }

    uint32_t STRIPEID = 0;
    uint32_t OFFSET = 0;
    LogicalBlkAddr lBlkAddr{
        .stripeId = STRIPEID,
        .offset = OFFSET};

    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = physicalSize.blksPerChunk,
        .buffers = &buffers};

    list<BufferEntry> parities;

    BufferEntry pParity = generateInitializedBufferEntry(NUM_BLOCKS, true);
    BufferEntry qParity = generateInitializedBufferEntry(NUM_BLOCKS, true);

    parities.push_back(pParity);
    parities.push_back(qParity);

    //Then
    raid6.TestParityGeneration(parities, *(src.buffers));
    memcpy(encoding_sources[dataCnt], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);
    memcpy(encoding_sources[dataCnt + 1], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);

    uint32_t tempInx = 0;
    vector<uint32_t> idx_for_test;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            idx_for_test.push_back(tempInx);
        }
        tempInx++;
    }

    uint32_t src_i = 0;
    for (auto idx : idx_for_test)
    {
        for (uint32_t j = 0; j < chunkSize; j++)
        {
            encoding_results_with_errors[src_i * chunkSize + j] = encoding_sources[idx][j];
        }
        src_i++;
    }

    vector<uint32_t> targets;
    targets.push_back(firstErrIdx);
    targets.push_back(secondErrIdx);
    raid6.TestRebuildData(decoding_results, encoding_results_with_errors, chunkSize, targets, abnormals);

    for (uint32_t i = 0; i < targets.size(); i++)
    {
        ASSERT_EQ(0, memcmp(decoding_results + i * chunkSize, encoding_sources[targets[i]], chunkSize));
    }
}

TEST(Raid6, Raid6_testTwoDeviceRebuildwithTwoDataDeviceErrorsCase02)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .lastLba = 0 /* not interesting */,
        .blksPerChunk = 64,
        .chunksPerStripe = 14,
        .stripesPerSegment = 0 /* not interesting */,
        .totalSegments = 0 /* not interesting */
    };

    Raid6 raid6(&physicalSize, 0);

    vector<uint32_t> abnormals;
    uint32_t firstErrIdx = 4;
    uint32_t secondErrIdx = 10;

    abnormals.push_back(firstErrIdx);
    abnormals.push_back(secondErrIdx);
    uint32_t nerrs = abnormals.size();

    // When
    uint32_t chunkCnt = physicalSize.chunksPerStripe;
    uint32_t chunkSize = physicalSize.blksPerChunk;
    uint32_t parityCnt = 2;
    uint32_t dataCnt = chunkCnt - parityCnt;
    uint32_t rebuildCnt = chunkCnt - nerrs;

    unsigned char encoding_sources[chunkCnt][chunkSize];
    unsigned char encoding_results_with_errors[chunkSize * rebuildCnt];
    unsigned char decoding_results[chunkSize * nerrs];

    list<BufferEntry> buffers;

    int NUM_BLOCKS = physicalSize.blksPerChunk;

    for (uint32_t i = 0; i < dataCnt; i++)
    {
        buffers.push_back(generateRandomBufferEntry(NUM_BLOCKS, false));
    }

    uint32_t iter = 0;
    unsigned char* src_ptr = nullptr;
    for (const BufferEntry& src_buffer : buffers)
    {
        src_ptr = (unsigned char*)src_buffer.GetBufferPtr();
        memcpy(encoding_sources[iter++], src_ptr, chunkSize);
    }

    uint32_t STRIPEID = 0;
    uint32_t OFFSET = 0;
    LogicalBlkAddr lBlkAddr{
        .stripeId = STRIPEID,
        .offset = OFFSET};

    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = physicalSize.blksPerChunk,
        .buffers = &buffers};

    list<BufferEntry> parities;

    BufferEntry pParity = generateInitializedBufferEntry(NUM_BLOCKS, true);
    BufferEntry qParity = generateInitializedBufferEntry(NUM_BLOCKS, true);

    parities.push_back(pParity);
    parities.push_back(qParity);

    //Then
    raid6.TestParityGeneration(parities, *(src.buffers));
    memcpy(encoding_sources[dataCnt], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);
    memcpy(encoding_sources[dataCnt + 1], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);

    uint32_t tempInx = 0;
    vector<uint32_t> idx_for_test;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            idx_for_test.push_back(tempInx);
        }
        tempInx++;
    }

    uint32_t src_i = 0;
    for (auto idx : idx_for_test)
    {
        for (uint32_t j = 0; j < chunkSize; j++)
        {
            encoding_results_with_errors[src_i * chunkSize + j] = encoding_sources[idx][j];
        }
        src_i++;
    }

    vector<uint32_t> targets;
    targets.push_back(firstErrIdx);
    targets.push_back(secondErrIdx);
    raid6.TestRebuildData(decoding_results, encoding_results_with_errors, chunkSize, targets, abnormals);

    for (uint32_t i = 0; i < targets.size(); i++)
    {
        ASSERT_EQ(0, memcmp(decoding_results + i * chunkSize, encoding_sources[targets[i]], chunkSize));
    }
}

TEST(Raid6, Raid6_testTwoDeviceRebuildwithOneDataDeviceandOneParityDeviceErrorsCase01)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .lastLba = 0 /* not interesting */,
        .blksPerChunk = 64,
        .chunksPerStripe = 14,
        .stripesPerSegment = 0 /* not interesting */,
        .totalSegments = 0 /* not interesting */
    };

    Raid6 raid6(&physicalSize, 0);

    vector<uint32_t> abnormals;
    uint32_t firstErrIdx = 10;
    uint32_t secondErrIdx = physicalSize.chunksPerStripe - 2;

    abnormals.push_back(firstErrIdx);
    abnormals.push_back(secondErrIdx);
    uint32_t nerrs = abnormals.size();

    // When
    uint32_t chunkCnt = physicalSize.chunksPerStripe;
    uint32_t chunkSize = physicalSize.blksPerChunk;
    uint32_t parityCnt = 2;
    uint32_t dataCnt = chunkCnt - parityCnt;
    uint32_t rebuildCnt = chunkCnt - nerrs;

    unsigned char encoding_sources[chunkCnt][chunkSize];
    unsigned char encoding_results_with_errors[chunkSize * rebuildCnt];
    unsigned char decoding_results[chunkSize * nerrs];

    list<BufferEntry> buffers;

    int NUM_BLOCKS = physicalSize.blksPerChunk;

    for (uint32_t i = 0; i < dataCnt; i++)
    {
        buffers.push_back(generateRandomBufferEntry(NUM_BLOCKS, false));
    }

    uint32_t iter = 0;
    unsigned char* src_ptr = nullptr;
    for (const BufferEntry& src_buffer : buffers)
    {
        src_ptr = (unsigned char*)src_buffer.GetBufferPtr();
        memcpy(encoding_sources[iter++], src_ptr, chunkSize);
    }

    uint32_t STRIPEID = 0;
    uint32_t OFFSET = 0;
    LogicalBlkAddr lBlkAddr{
        .stripeId = STRIPEID,
        .offset = OFFSET};

    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = physicalSize.blksPerChunk,
        .buffers = &buffers};

    list<BufferEntry> parities;

    BufferEntry pParity = generateInitializedBufferEntry(NUM_BLOCKS, true);
    BufferEntry qParity = generateInitializedBufferEntry(NUM_BLOCKS, true);

    parities.push_back(pParity);
    parities.push_back(qParity);

    //Then
    raid6.TestParityGeneration(parities, *(src.buffers));
    memcpy(encoding_sources[dataCnt], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);
    memcpy(encoding_sources[dataCnt + 1], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);

    uint32_t tempInx = 0;
    vector<uint32_t> idx_for_test;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            idx_for_test.push_back(tempInx);
        }
        tempInx++;
    }

    uint32_t src_i = 0;
    for (auto idx : idx_for_test)
    {
        for (uint32_t j = 0; j < chunkSize; j++)
        {
            encoding_results_with_errors[src_i * chunkSize + j] = encoding_sources[idx][j];
        }
        src_i++;
    }

    vector<uint32_t> targets;
    targets.push_back(firstErrIdx);
    targets.push_back(secondErrIdx);
    raid6.TestRebuildData(decoding_results, encoding_results_with_errors, chunkSize, targets, abnormals);

    for (uint32_t i = 0; i < targets.size(); i++)
    {
        ASSERT_EQ(0, memcmp(decoding_results + i * chunkSize, encoding_sources[targets[i]], chunkSize));
    }
}

TEST(Raid6, Raid6_testTwoDeviceRebuildwithOneDataDeviceandOneParityDeviceErrorsCase02)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .lastLba = 0 /* not interesting */,
        .blksPerChunk = 64,
        .chunksPerStripe = 14,
        .stripesPerSegment = 0 /* not interesting */,
        .totalSegments = 0 /* not interesting */
    };

    Raid6 raid6(&physicalSize, 0);

    vector<uint32_t> abnormals;
    uint32_t firstErrIdx = 2;
    uint32_t secondErrIdx = physicalSize.chunksPerStripe - 1;

    abnormals.push_back(firstErrIdx);
    abnormals.push_back(secondErrIdx);
    uint32_t nerrs = abnormals.size();

    // When
    uint32_t chunkCnt = physicalSize.chunksPerStripe;
    uint32_t chunkSize = physicalSize.blksPerChunk;
    uint32_t parityCnt = 2;
    uint32_t dataCnt = chunkCnt - parityCnt;
    uint32_t rebuildCnt = chunkCnt - nerrs;

    unsigned char encoding_sources[chunkCnt][chunkSize];
    unsigned char encoding_results_with_errors[chunkSize * rebuildCnt];
    unsigned char decoding_results[chunkSize * nerrs];

    list<BufferEntry> buffers;

    int NUM_BLOCKS = physicalSize.blksPerChunk;

    for (uint32_t i = 0; i < dataCnt; i++)
    {
        buffers.push_back(generateRandomBufferEntry(NUM_BLOCKS, false));
    }

    uint32_t iter = 0;
    unsigned char* src_ptr = nullptr;
    for (const BufferEntry& src_buffer : buffers)
    {
        src_ptr = (unsigned char*)src_buffer.GetBufferPtr();
        memcpy(encoding_sources[iter++], src_ptr, chunkSize);
    }

    uint32_t STRIPEID = 0;
    uint32_t OFFSET = 0;
    LogicalBlkAddr lBlkAddr{
        .stripeId = STRIPEID,
        .offset = OFFSET};

    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = physicalSize.blksPerChunk,
        .buffers = &buffers};

    list<BufferEntry> parities;

    BufferEntry pParity = generateInitializedBufferEntry(NUM_BLOCKS, true);
    BufferEntry qParity = generateInitializedBufferEntry(NUM_BLOCKS, true);

    parities.push_back(pParity);
    parities.push_back(qParity);

    //Then
    raid6.TestParityGeneration(parities, *(src.buffers));
    memcpy(encoding_sources[dataCnt], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);
    memcpy(encoding_sources[dataCnt + 1], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);

    uint32_t tempInx = 0;
    vector<uint32_t> idx_for_test;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            idx_for_test.push_back(tempInx);
        }
        tempInx++;
    }

    uint32_t src_i = 0;
    for (auto idx : idx_for_test)
    {
        for (uint32_t j = 0; j < chunkSize; j++)
        {
            encoding_results_with_errors[src_i * chunkSize + j] = encoding_sources[idx][j];
        }
        src_i++;
    }

    vector<uint32_t> targets;
    targets.push_back(firstErrIdx);
    targets.push_back(secondErrIdx);
    raid6.TestRebuildData(decoding_results, encoding_results_with_errors, chunkSize, targets, abnormals);

    for (uint32_t i = 0; i < targets.size(); i++)
    {
        ASSERT_EQ(0, memcmp(decoding_results + i * chunkSize, encoding_sources[targets[i]], chunkSize));
    }
}

TEST(Raid6, Raid6_testTwoDeviceRebuildwithTwoParityDeviceErrorsCase01)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .lastLba = 0 /* not interesting */,
        .blksPerChunk = 64,
        .chunksPerStripe = 14,
        .stripesPerSegment = 0 /* not interesting */,
        .totalSegments = 0 /* not interesting */
    };

    Raid6 raid6(&physicalSize, 0);

    vector<uint32_t> abnormals;
    uint32_t firstErrIdx = physicalSize.chunksPerStripe - 2;
    uint32_t secondErrIdx = physicalSize.chunksPerStripe - 1;

    abnormals.push_back(firstErrIdx);
    abnormals.push_back(secondErrIdx);
    uint32_t nerrs = abnormals.size();

    // When
    uint32_t chunkCnt = physicalSize.chunksPerStripe;
    uint32_t chunkSize = physicalSize.blksPerChunk;
    uint32_t parityCnt = 2;
    uint32_t dataCnt = chunkCnt - parityCnt;
    uint32_t rebuildCnt = chunkCnt - nerrs;

    unsigned char encoding_sources[chunkCnt][chunkSize];
    unsigned char encoding_results_with_errors[chunkSize * rebuildCnt];
    unsigned char decoding_results[chunkSize * nerrs];

    list<BufferEntry> buffers;

    int NUM_BLOCKS = physicalSize.blksPerChunk;

    for (uint32_t i = 0; i < dataCnt; i++)
    {
        buffers.push_back(generateRandomBufferEntry(NUM_BLOCKS, false));
    }

    uint32_t iter = 0;
    unsigned char* src_ptr = nullptr;
    for (const BufferEntry& src_buffer : buffers)
    {
        src_ptr = (unsigned char*)src_buffer.GetBufferPtr();
        memcpy(encoding_sources[iter++], src_ptr, chunkSize);
    }

    uint32_t STRIPEID = 0;
    uint32_t OFFSET = 0;
    LogicalBlkAddr lBlkAddr{
        .stripeId = STRIPEID,
        .offset = OFFSET};

    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = physicalSize.blksPerChunk,
        .buffers = &buffers};

    list<BufferEntry> parities;

    BufferEntry pParity = generateInitializedBufferEntry(NUM_BLOCKS, true);
    BufferEntry qParity = generateInitializedBufferEntry(NUM_BLOCKS, true);

    parities.push_back(pParity);
    parities.push_back(qParity);

    //Then
    raid6.TestParityGeneration(parities, *(src.buffers));
    memcpy(encoding_sources[dataCnt], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);
    memcpy(encoding_sources[dataCnt + 1], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);

    uint32_t tempInx = 0;
    vector<uint32_t> idx_for_test;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            idx_for_test.push_back(tempInx);
        }
        tempInx++;
    }

    uint32_t src_i = 0;
    for (auto idx : idx_for_test)
    {
        for (uint32_t j = 0; j < chunkSize; j++)
        {
            encoding_results_with_errors[src_i * chunkSize + j] = encoding_sources[idx][j];
        }
        src_i++;
    }

    vector<uint32_t> targets;
    targets.push_back(firstErrIdx);
    targets.push_back(secondErrIdx);
    raid6.TestRebuildData(decoding_results, encoding_results_with_errors, chunkSize, targets, abnormals);

    for (uint32_t i = 0; i < targets.size(); i++)
    {
        ASSERT_EQ(0, memcmp(decoding_results + i * chunkSize, encoding_sources[targets[i]], chunkSize));
    }
}

TEST(Raid6, Raid6_testTwoDeviceRebuildwithTwoParityDeviceErrorsCase02)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0 /* not interesting */,
        .lastLba = 0 /* not interesting */,
        .blksPerChunk = 64,
        .chunksPerStripe = 14,
        .stripesPerSegment = 0 /* not interesting */,
        .totalSegments = 0 /* not interesting */
    };

    Raid6 raid6(&physicalSize, 0);

    vector<uint32_t> abnormals;
    uint32_t firstErrIdx = physicalSize.chunksPerStripe - 2;
    uint32_t secondErrIdx = physicalSize.chunksPerStripe - 1;

    abnormals.push_back(firstErrIdx);
    abnormals.push_back(secondErrIdx);
    uint32_t nerrs = abnormals.size();

    // When
    uint32_t chunkCnt = physicalSize.chunksPerStripe;
    uint32_t chunkSize = physicalSize.blksPerChunk;
    uint32_t parityCnt = 2;
    uint32_t dataCnt = chunkCnt - parityCnt;
    uint32_t rebuildCnt = chunkCnt - nerrs;

    unsigned char encoding_sources[chunkCnt][chunkSize];
    unsigned char encoding_results_with_errors[chunkSize * rebuildCnt];
    unsigned char decoding_results[chunkSize * nerrs];

    list<BufferEntry> buffers;

    int NUM_BLOCKS = physicalSize.blksPerChunk;

    for (uint32_t i = 0; i < dataCnt; i++)
    {
        buffers.push_back(generateRandomBufferEntry(NUM_BLOCKS, false));
    }

    uint32_t iter = 0;
    unsigned char* src_ptr = nullptr;
    for (const BufferEntry& src_buffer : buffers)
    {
        src_ptr = (unsigned char*)src_buffer.GetBufferPtr();
        memcpy(encoding_sources[iter++], src_ptr, chunkSize);
    }

    uint32_t STRIPEID = 3;
    uint32_t OFFSET = 12;
    LogicalBlkAddr lBlkAddr{
        .stripeId = STRIPEID,
        .offset = OFFSET};

    const LogicalWriteEntry& src{
        .addr = lBlkAddr,
        .blkCnt = physicalSize.blksPerChunk,
        .buffers = &buffers};

    list<BufferEntry> parities;

    BufferEntry pParity = generateInitializedBufferEntry(NUM_BLOCKS, true);
    BufferEntry qParity = generateInitializedBufferEntry(NUM_BLOCKS, true);

    parities.push_back(pParity);
    parities.push_back(qParity);

    //Then
    raid6.TestParityGeneration(parities, *(src.buffers));
    memcpy(encoding_sources[dataCnt], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);
    memcpy(encoding_sources[dataCnt + 1], (unsigned char*)parities.front().GetBufferPtr(), chunkSize);

    uint32_t tempInx = 0;
    vector<uint32_t> idx_for_test;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            idx_for_test.push_back(tempInx);
        }
        tempInx++;
    }

    uint32_t src_i = 0;
    for (auto idx : idx_for_test)
    {
        for (uint32_t j = 0; j < chunkSize; j++)
        {
            encoding_results_with_errors[src_i * chunkSize + j] = encoding_sources[idx][j];
        }
        src_i++;
    }

    vector<uint32_t> targets;
    targets.push_back(firstErrIdx);
    targets.push_back(secondErrIdx);
    raid6.TestRebuildData(decoding_results, encoding_results_with_errors, chunkSize, targets, abnormals);

    for (uint32_t i = 0; i < targets.size(); i++)
    {
        ASSERT_EQ(0, memcmp(decoding_results + i * chunkSize, encoding_sources[targets[i]], chunkSize));
    }
}
} // namespace pos
