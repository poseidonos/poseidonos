#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <fstream>

#include "src/pbr/load/pbr_file_loader.h"
#include "src/pbr/update/pbr_file_updater.h"
#include "src/pbr/io/pbr_reader.h"
#include "src/pbr/content/fake_revision/content_serializer_fake_revision.h"
#include "src/pbr/content/fake_revision/fake_ate_data.h"

namespace pbr
{
TEST(PbrSerializerTest, Serialize_testIfResultsOfSerializingSamplePbr)
{
    // 1. Load(Deserialize) PBR from a sample pbr file.
    // 2. Write(Serializize) the PBR to a test pbr file(without any change).
    // 3. Compare whether sample file and test file are the same.

    // Given
    uint32_t revision = 0;
    uint32_t fileSize = 64 * 1024; // 64KB PBR
    char buff[FILENAME_MAX];
    getcwd(buff, FILENAME_MAX);
    string dirPath(buff);
    string samplePbr = dirPath + "/integration-tests/pbr/sample_pbr.pbr";
    string testPbr = dirPath + "/integration-tests/pbr/test_pbr.pbr";
    unique_ptr<IPbrLoader> loader = make_unique<PbrFileLoader>(vector<string>{ samplePbr });
    unique_ptr<IPbrUpdater> updater = make_unique<PbrFileUpdater>(revision, testPbr);
    vector<unique_ptr<AteData>> ateList;

    // When
    int ret = loader->Load(ateList);
    ASSERT_EQ(1, ateList.size());
    ASSERT_EQ(0, ret);

    auto& ateData = ateList.front();
    ret = updater->Update(ateData.get());
    ASSERT_EQ(0, ret);

    // Read original sample to compare
    char samplePbrData[fileSize];
    ifstream f(samplePbr, ios::in | ios::ate);
    f.seekg(0, ios::beg);
    f.read(samplePbrData, fileSize);
    f.close();

    char testPbrData[fileSize];
    IPbrReader* testPbrReader = new PbrReader();
    testPbrReader->Read(testPbr, testPbrData, 0, fileSize);

    // Then
    int i;
    for (i = 0; i < fileSize; i++)
    {
        if (samplePbrData[i] != testPbrData[i])
        {
            break;
        }
    }
    ASSERT_EQ(i, fileSize);

    // Clean Up
    updater->Clear(); // remove test_pbr.pbr
    delete testPbrReader;
}

TEST(PbrSerializerTest, Serialize_testIfPbrRevision0IsUpdatedWellToNextPbr)
{
    // 1. Load(Deserialize) revision-0 PBR from a sample pbr file.
    // 2. Add Fake PBR signature.
    // 3. Update(Serializize) to a new PBR revision-uint32_max that includes the Fake PBR signature
    // 4. Load and verify the updated PBR

    // Given
    uint32_t newRevision = UINT32_MAX;
    uint32_t fileSize = 64 * 1024; // 64KB PBR
    char buff[FILENAME_MAX];
    getcwd(buff, FILENAME_MAX);
    string dirPath(buff);
    string samplePbr = dirPath + "/integration-tests/pbr/sample_pbr.pbr";
    string newPbr = dirPath + "/integration-tests/pbr/new_pbr.pbr";
    unique_ptr<IPbrLoader> originPbrLoader = make_unique<PbrFileLoader>(vector<string>{ samplePbr });
    unique_ptr<IPbrUpdater> updater = make_unique<PbrFileUpdater>(newRevision, newPbr);
    vector<unique_ptr<AteData>> ateList;

    // When
    // 1. Load(Deserialize) revision-0 PBR from a sample pbr file.
    int ret = originPbrLoader->Load(ateList);
    ASSERT_EQ(1, ateList.size());
    ASSERT_EQ(0, ret);
    auto originData = move(ateList.front());
    FakeAteData* fakeData = new FakeAteData(*originData);
    ateList.clear();

    // 2. Add Fake PBR signature.
    fakeData->fakeSignature = "FAKE";

    // 3. Update(Serializize) to a new PBR revision-uint32_max that includes the Fake PBR signature
    ret = updater->Update(fakeData);
    ASSERT_EQ(0, ret);

    // Then
    unique_ptr<IPbrLoader> newLoader = make_unique<PbrFileLoader>(vector<string>{ newPbr });
    ret = newLoader->Load(ateList);
    ASSERT_EQ(1, ateList.size());
    ASSERT_EQ(0, ret);
    auto reloadedFakeData = dynamic_cast<FakeAteData*>(move(ateList.front()).get());
    ASSERT_EQ(reloadedFakeData->fakeSignature, fakeData->fakeSignature);
    ASSERT_EQ(reloadedFakeData->nodeUuid, fakeData->nodeUuid);
    ASSERT_EQ(reloadedFakeData->arrayName, fakeData->arrayName);
    ASSERT_EQ(reloadedFakeData->arrayUuid, fakeData->arrayUuid);
    ASSERT_EQ(reloadedFakeData->createdDateTime, fakeData->createdDateTime);
    ASSERT_EQ(reloadedFakeData->lastUpdatedDateTime, fakeData->lastUpdatedDateTime);
    ASSERT_EQ(reloadedFakeData->adeList.size(), fakeData->adeList.size());
    ASSERT_EQ(reloadedFakeData->pteList.size(), fakeData->pteList.size());
    for (size_t i = 0; i < reloadedFakeData->adeList.size(); i++)
    {
        ASSERT_EQ(reloadedFakeData->adeList[i]->devIndex,
            fakeData->adeList[i]->devIndex);
        ASSERT_EQ(reloadedFakeData->adeList[i]->devSn,
            fakeData->adeList[i]->devSn);
        ASSERT_EQ(reloadedFakeData->adeList[i]->devState,
            fakeData->adeList[i]->devState);
        ASSERT_EQ(reloadedFakeData->adeList[i]->devType,
            fakeData->adeList[i]->devType);
    }

    for (size_t i = 0; i < reloadedFakeData->pteList.size(); i++)
    {
        ASSERT_EQ(reloadedFakeData->pteList[i]->partType,
            fakeData->pteList[i]->partType);
        ASSERT_EQ(reloadedFakeData->pteList[i]->raidType,
            fakeData->pteList[i]->raidType);
        ASSERT_EQ(reloadedFakeData->pteList[i]->startLba,
            fakeData->pteList[i]->startLba);
        ASSERT_EQ(reloadedFakeData->pteList[i]->lastLba,
            fakeData->pteList[i]->lastLba);
    }

    // Clean Up
    ateList.clear();
    updater->Clear(); // remove test0_pbr.pbr
}
}  // namespace pbr
