#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/vsamap/vsamap_manager.h"

namespace pos
{
class MockVSAMapManager : public VSAMapManager
{
public:
    using VSAMapManager::VSAMapManager;
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(int, CreateVsaMapContent, (VSAMapContent * vsaMap, int volId, uint64_t volSizeByte, bool delVol), (override));
    MOCK_METHOD(int, LoadVSAMapFile, (int volId), (override));
    MOCK_METHOD(int, FlushDirtyPagesGiven, (int volId, MpageList list, EventSmartPtr cb), (override));
    MOCK_METHOD(int, FlushTouchedPages, (int volId, EventSmartPtr cb), (override));
    MOCK_METHOD(int, FlushAllMaps, (), (override));
    MOCK_METHOD(void, WaitAllPendingIoDone, (), (override));
    MOCK_METHOD(void, WaitLoadPendingIoDone, (), (override));
    MOCK_METHOD(void, WaitWritePendingIoDone, (), (override));
    MOCK_METHOD(void, WaitVolumePendingIoDone, (int volId), (override));
    MOCK_METHOD(bool, IsVolumeLoaded, (int volId), (override));
    MOCK_METHOD(void, MapFlushDone, (int mapId), (override));
    MOCK_METHOD(int, GetVSAs, (int volumeId, BlkAddr startRba, uint32_t numBlks, VsaArray& vsaArray), (override));
    MOCK_METHOD(int, SetVSAs, (int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks), (override));
    MOCK_METHOD(VirtualBlkAddr, GetRandomVSA, (BlkAddr rba), (override));
    MOCK_METHOD(int64_t, GetNumUsedBlks, (int volId), (override));
    MOCK_METHOD(VirtualBlkAddr, GetVSAWoCond, (int volumeId, BlkAddr rba), (override));
    MOCK_METHOD(int, SetVSAsWoCond, (int volumeId, BlkAddr startfRba, VirtualBlks& virtualBlks), (override));
    MOCK_METHOD(MpageList, GetDirtyVsaMapPages, (int volId, BlkAddr startRba, uint64_t numBlks), (override));
    MOCK_METHOD(VSAMapContent*, GetVSAMapContent, (int volId), (override));
    MOCK_METHOD(void, SetVSAMapContent, (int volId, VSAMapContent* content), (override));
    MOCK_METHOD(bool, NeedToDeleteFile, (int volId), (override));
    MOCK_METHOD(int, InvalidateAllBlocks, (int volId, ISegmentCtx* segmentCtx), (override));
    MOCK_METHOD(int, DeleteVSAMap, (int volId), (override));
    MOCK_METHOD(bool, IsVsaMapAccessible, (int volId), (override));
    MOCK_METHOD(void, EnableVsaMapAccess, (int volId), (override));
    MOCK_METHOD(void, DisableVsaMapAccess, (int volId), (override));
    MOCK_METHOD(bool, IsVsaMapInternalAccesible, (int volId), (override));
    MOCK_METHOD(void, EnableVsaMapInternalAccess, (int volId), (override));
    MOCK_METHOD(void, DisableVsaMapInternalAccess, (int volId), (override));
    MOCK_METHOD(int, Dump, (int volId, std::string fileName), (override));
    MOCK_METHOD(int, DumpLoad, (int volId, std::string fileName), (override));
};

} // namespace pos
