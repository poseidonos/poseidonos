#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/storage/pstore/mss_on_disk.h"

namespace pos
{
class MockMssIoCompletion : public MssIoCompletion
{
public:
    using MssIoCompletion::MssIoCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

class MockMssOnDisk : public MssOnDisk
{
public:
    using MssOnDisk::MssOnDisk;
    MOCK_METHOD(POS_EVENT_ID, CreateMetaStore, (int arrayId, MetaStorageType mediaType, uint64_t capacity, bool formatFlag), (override));
    MOCK_METHOD(POS_EVENT_ID, Open, (), (override));
    MOCK_METHOD(POS_EVENT_ID, Close, (), (override));
    MOCK_METHOD(uint64_t, GetCapacity, (MetaStorageType mediaType), (override));
    MOCK_METHOD(POS_EVENT_ID, ReadPage, (MetaStorageType mediaType, MetaLpnType pageNumber, void* buffer, MetaLpnType numPages), (override));
    MOCK_METHOD(POS_EVENT_ID, WritePage, (MetaStorageType mediaType, MetaLpnType pageNumber, void* buffer, MetaLpnType numPages), (override));
    MOCK_METHOD(bool, IsAIOSupport, (), (override));
    MOCK_METHOD(POS_EVENT_ID, ReadPageAsync, (MssAioCbCxt * cb), (override));
    MOCK_METHOD(POS_EVENT_ID, WritePageAsync, (MssAioCbCxt * cb), (override));
    MOCK_METHOD(POS_EVENT_ID, TrimFileData, (MetaStorageType mediaType, MetaLpnType startLpn, void* buffer, MetaLpnType numPages), (override));
};

} // namespace pos
