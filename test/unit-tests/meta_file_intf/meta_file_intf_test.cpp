#include "src/meta_file_intf/meta_file_intf.h"

#include <gtest/gtest.h>

namespace pos
{
class MetaFileIntfTester : public MetaFileIntf
{
public:
    MetaFileIntfTester(void)
    : MetaFileIntf()
    {
    }
    MetaFileIntfTester(std::string fname, std::string aname,
                        StorageOpt storageOpt = StorageOpt::DEFAULT)
    : MetaFileIntf(fname, aname, storageOpt)
    {
    }
    MetaFileIntfTester(std::string fname, int arrayId,
                        StorageOpt storageOpt = StorageOpt::DEFAULT)
    : MetaFileIntf(fname, arrayId, storageOpt)
    {
    }
    ~MetaFileIntfTester(void)
    {
    }
    int Create(uint64_t size)
    {
        return 0;
    }
    bool DoesFileExist(void)
    {
        return 0;
    }
    int Delete(void)
    {
        return 0;
    }
    uint64_t GetFileSize(void)
    {
        return 0;
    }
    int AsyncIO(AsyncMetaFileIoCtx* ctx)
    {
        return 0;
    }
    int CheckIoDoneStatus(void* data)
    {
        return 0;
    }
    int _Read(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
    {
        return 1;
    }
    int _Write(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
    {
        return 1;
    }

    void SetFd(int fd)
    {
        this->fd = fd;
    }
};

TEST(MetaFileIntfTester, CheckFileName)
{
    std::string fileName = "TESTFILE";
    int arrayId = 0;
    MetaFileIntfTester* file = new MetaFileIntfTester(fileName, 0);

    EXPECT_EQ(fileName, file->GetFileName());

    delete file;
}

TEST(MetaFileIntfTester, CheckFileDescriptor)
{
    std::string fileName = "TESTFILE";
    std::string arrayName = "TESTARRAY";
    MetaFileIntfTester* file = new MetaFileIntfTester();

    file->SetFd(100);

    EXPECT_EQ(100, file->GetFd());

    delete file;
}

TEST(MetaFileIntfTester, CheckIssueIO_Read)
{
    MetaFileIntfTester* file = new MetaFileIntfTester();

    MetaFsIoOpcode opType = MetaFsIoOpcode::Read;
    uint64_t fileOffset = 20;
    uint64_t length = 10;
    char* buffer = nullptr;

    EXPECT_EQ(1, file->IssueIO(opType, fileOffset, length, buffer));

    delete file;
}

TEST(MetaFileIntfTester, CheckIssueIO_Write)
{
    MetaFileIntfTester* file = new MetaFileIntfTester();

    MetaFsIoOpcode opType = MetaFsIoOpcode::Write;
    uint64_t fileOffset = 20;
    uint64_t length = 10;
    char* buffer = nullptr;

    EXPECT_EQ(1, file->IssueIO(opType, fileOffset, length, buffer));

    delete file;
}

TEST(MetaFileIntfTester, CheckAppendIO_Read)
{
    MetaFileIntfTester* file = new MetaFileIntfTester();

    MetaFsIoOpcode opType = MetaFsIoOpcode::Read;
    uint64_t fileOffset = 20;
    uint64_t length = 10;
    char* buffer = nullptr;

    EXPECT_EQ(1, file->AppendIO(opType, fileOffset, length, buffer));

    delete file;
}

TEST(MetaFileIntfTester, CheckAppendIO_Write)
{
    MetaFileIntfTester* file = new MetaFileIntfTester();

    MetaFsIoOpcode opType = MetaFsIoOpcode::Write;
    uint64_t fileOffset = 20;
    uint64_t length = 10;
    char* buffer = nullptr;

    EXPECT_EQ(1, file->AppendIO(opType, fileOffset, length, buffer));

    delete file;
}
} // namespace pos
