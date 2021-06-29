#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/storage/vstore/mss_ramdisk.h"

namespace pos
{
class MockMssRamdisk : public MssRamdisk
{
public:
    using MssRamdisk::MssRamdisk;
    MOCK_METHOD(POS_EVENT_ID, CreateMetaStore, (int arrayId, MetaStorageType mediaType, uint64_t capacity, bool formatFlag), (override));
    MOCK_METHOD(POS_EVENT_ID, Open, (), (override));
    MOCK_METHOD(POS_EVENT_ID, Close, (), (override));
    MOCK_METHOD(POS_EVENT_ID, ReadPage, (MetaStorageType mediaType, MetaLpnType metaLpn, void* buffer, MetaLpnType numPages), (override));
    MOCK_METHOD(POS_EVENT_ID, WritePage, (MetaStorageType mediaType, MetaLpnType metaLpn, void* buffer, MetaLpnType numPages), (override));
    MOCK_METHOD(bool, IsAIOSupport, (), (override));
    MOCK_METHOD(POS_EVENT_ID, ReadPageAsync, (MssAioCbCxt * cb), (override));
    MOCK_METHOD(POS_EVENT_ID, WritePageAsync, (MssAioCbCxt * cb), (override));
    MOCK_METHOD(POS_EVENT_ID, TrimFileData, (MetaStorageType mediaType, MetaLpnType startLpn, void* buffer, MetaLpnType numPages), (override));
};

} // namespace pos
