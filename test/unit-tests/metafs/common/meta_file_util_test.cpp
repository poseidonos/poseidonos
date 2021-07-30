#include "src/metafs/common/meta_file_util.h"

#include <string>
#include <gtest/gtest.h>

namespace pos
{
TEST(MetaFileUtil, Convert_FileName_To_HashKey)
{
    std::string fileName = "TESTFILE";
    StringHashType expect = std::hash<std::string>{}(fileName.c_str());
    StringHashType result = MetaFileUtil::GetHashKeyFromFileName(fileName);

    EXPECT_EQ(result, expect);
}

TEST(MetaFileUtil, Convert_MetaVolumeType_To_MetaStorageType)
{
    MetaStorageType storageType = MetaStorageType::Default;

    storageType = MetaFileUtil::ConvertToMediaType(MetaVolumeType::NvRamVolume);
    EXPECT_EQ(storageType, MetaStorageType::NVRAM);

    storageType = MetaFileUtil::ConvertToMediaType(MetaVolumeType::SsdVolume);
    EXPECT_EQ(storageType, MetaStorageType::SSD);
}

TEST(MetaFileUtil, Convert_StorageOpt_To_MetaStorageType)
{
    MetaStorageType storageType = MetaStorageType::Default;

    storageType = MetaFileUtil::ConvertToMediaType(StorageOpt::NVRAM);
    EXPECT_EQ(storageType, MetaStorageType::NVRAM);

    storageType = MetaFileUtil::ConvertToMediaType(StorageOpt::SSD);
    EXPECT_EQ(storageType, MetaStorageType::SSD);
}

TEST(MetaFileUtil, Convert_MetaVolumeType_To_String)
{
    std::string result = "";

    result = MetaFileUtil::ConvertToMediaTypeName(MetaVolumeType::NvRamVolume);
    EXPECT_EQ(result, std::string("NVRAM/NVDIMM"));

    result = MetaFileUtil::ConvertToMediaTypeName(MetaVolumeType::SsdVolume);
    EXPECT_EQ(result, std::string("SSD array"));
}

TEST(MetaFileUtil, Convert_MetaStorageType_To_MetaVolumeType)
{
    MetaVolumeType volumeType = MetaVolumeType::Max;

    volumeType = MetaFileUtil::ConvertToVolumeType(MetaStorageType::NVRAM);
    EXPECT_EQ(volumeType, MetaVolumeType::NvRamVolume);

    volumeType = MetaFileUtil::ConvertToVolumeType(MetaStorageType::SSD);
    EXPECT_EQ(volumeType, MetaVolumeType::SsdVolume);
}

TEST(MetaFileUtil, Convert_StorageOpt_To_MetaVolumeType)
{
    MetaVolumeType volumeType = MetaVolumeType::Max;

    volumeType = MetaFileUtil::ConvertToVolumeType(StorageOpt::NVRAM);
    EXPECT_EQ(volumeType, MetaVolumeType::NvRamVolume);

    volumeType = MetaFileUtil::ConvertToVolumeType(StorageOpt::SSD);
    EXPECT_EQ(volumeType, MetaVolumeType::SsdVolume);
}

TEST(MetaFileUtil, Check_EpochSignature)
{
    std::time_t t = std::time(0);

    // from the method
    std::tm* now = std::localtime(&t);
    uint64_t sign = (uint64_t)(now->tm_year + 1900) * 10000000000 +
        (uint64_t)(now->tm_mon + 1) * 100000000 +
        (uint64_t)now->tm_mday * 1000000 +
        (uint64_t)now->tm_hour * 10000 +
        (uint64_t)now->tm_min * 100 +
        (uint64_t)now->tm_sec;

    uint64_t result = MetaFileUtil::GetEpochSignature(t);
    std::cout << result << std::endl;
    EXPECT_EQ(sign, result);

    result = MetaFileUtil::GetEpochSignature();
    std::cout << result << std::endl;
    EXPECT_NE(0, result);
}
} // namespace pos
