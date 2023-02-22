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
    IPbrLoader* loader = new PbrFileLoader(vector<string>{ samplePbr });
    IPbrUpdater* updater = new PbrFileUpdater(revision, testPbr);
    vector<AteData*> ateList;

    // When
    int ret = loader->Load(ateList);
    ASSERT_EQ(1, ateList.size());
    ASSERT_EQ(0, ret);

    AteData* ateData = ateList.front();
    ret = updater->Update(ateData);
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
    delete updater;
    delete loader;
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
    IPbrLoader* oldLoader = new PbrFileLoader(vector<string>{ samplePbr });
    IPbrUpdater* updater = new PbrFileUpdater(newRevision, newPbr);
    vector<AteData*> ateList;

    // When
    // 1. Load(Deserialize) revision-0 PBR from a sample pbr file.
    int ret = oldLoader->Load(ateList);
    ASSERT_EQ(1, ateList.size());
    ASSERT_EQ(0, ret);
    AteData* oldAteData = ateList.front();
    FakeAteData* newAteData = new FakeAteData(oldAteData);

    // 2. Add Fake PBR signature.
    newAteData->fakeSignature = "FAKE";

    // 3. Update(Serializize) to a new PBR revision-uint32_max that includes the Fake PBR signature
    ret = updater->Update(newAteData);
    ASSERT_EQ(0, ret);

    // Then
    IPbrLoader* newLoader = new PbrFileLoader(vector<string>{ newPbr });
    ateList.clear();
    ret = newLoader->Load(ateList);
    ASSERT_EQ(1, ateList.size());
    ASSERT_EQ(0, ret);
    FakeAteData* reloadedNewAteData = dynamic_cast<FakeAteData*>(ateList.front());
    ASSERT_EQ(reloadedNewAteData->fakeSignature, newAteData->fakeSignature);
    ASSERT_EQ(reloadedNewAteData->nodeUuid, newAteData->nodeUuid);
    ASSERT_EQ(reloadedNewAteData->arrayName, newAteData->arrayName);
    ASSERT_EQ(reloadedNewAteData->arrayUuid, newAteData->arrayUuid);
    ASSERT_EQ(reloadedNewAteData->createdDateTime, newAteData->createdDateTime);
    ASSERT_EQ(reloadedNewAteData->lastUpdatedDateTime, newAteData->lastUpdatedDateTime);
    ASSERT_EQ(reloadedNewAteData->adeList.size(), newAteData->adeList.size());
    ASSERT_EQ(reloadedNewAteData->pteList.size(), newAteData->pteList.size());
    for (size_t i = 0; i < reloadedNewAteData->adeList.size(); i++)
    {
        ASSERT_EQ(reloadedNewAteData->adeList[i]->devIndex,
            newAteData->adeList[i]->devIndex);
        ASSERT_EQ(reloadedNewAteData->adeList[i]->devSn,
            newAteData->adeList[i]->devSn);
        ASSERT_EQ(reloadedNewAteData->adeList[i]->devState,
            newAteData->adeList[i]->devState);
        ASSERT_EQ(reloadedNewAteData->adeList[i]->devType,
            newAteData->adeList[i]->devType);
    }

    for (size_t i = 0; i < reloadedNewAteData->pteList.size(); i++)
    {
        ASSERT_EQ(reloadedNewAteData->pteList[i]->partType,
            newAteData->pteList[i]->partType);
        ASSERT_EQ(reloadedNewAteData->pteList[i]->raidType,
            newAteData->pteList[i]->raidType);
        ASSERT_EQ(reloadedNewAteData->pteList[i]->startLba,
            newAteData->pteList[i]->startLba);
        ASSERT_EQ(reloadedNewAteData->pteList[i]->lastLba,
            newAteData->pteList[i]->lastLba);
    }

    // Clean Up
    updater->Clear(); // remove test_pbr.pbr
    delete reloadedNewAteData;
    delete newLoader;
    delete newAteData;
    delete oldAteData;
    delete updater;
    delete oldLoader;
}
}  // namespace pbr
