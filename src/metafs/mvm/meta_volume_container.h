#pragma once

#include <map>
#include <string>
#include <utility>

#include "meta_storage_specific.h"
#include "meta_volume.h"
#include "meta_volume_type.h"
#include "os_header.h"

namespace pos
{
enum class BackupInfo
{
    First = 0,
    BaseLpn = First,
    CatalogSize,
    InodeHdrSize,
    InodeTableSize,
    Last = InodeTableSize,

    Max,
    Invalid = Max,
};

class MetaVolumeContainer
{
public:
    MetaVolumeContainer(void);
    ~MetaVolumeContainer(void);

    bool OpenAllVolumes(bool isNPOR);
    bool CloseAllVolumes(bool& resetContext /*output*/);
    void RegisterVolumeInstance(MetaVolumeType volType, MetaVolume* metaVol);
    void SetNvRamVolumeAvailable(void);
    bool IsNvRamVolumeAvailable(void);
    bool IsGivenVolumeExist(MetaVolumeType volumeType);
#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    bool Compaction(void);
#endif
    MetaVolume& GetMetaVolume(MetaVolumeType volumeType);
    std::pair<MetaVolumeType, POS_EVENT_ID> DetermineVolumeToCreateFile(FileSizeType fileByteSize, MetaFilePropertySet& prop);
    std::pair<MetaVolumeType, POS_EVENT_ID> LookupMetaVolumeType(FileDescriptorType fd);
    std::pair<MetaVolumeType, POS_EVENT_ID> LookupMetaVolumeType(std::string& fileName);
    void UpdateVolumeLookupInfo(StringHashType fileHashKey, FileDescriptorType fd, MetaVolumeType volumeType);
    void RemoveVolumeLookupInfo(StringHashType fileHashKey, FileDescriptorType fd);
    void BuildFreeFDMap(std::map<FileDescriptorType, FileDescriptorType>& dstFreeFDMap);
    void BuildFDLookup(std::unordered_map<StringHashType, FileDescriptorType>& fileKeyLookupMap);
    inline bool
    GetVolOpenFlag(void)
    {
        return allVolumesOpened;
    }

private:
    bool _CheckOkayToStore(MetaVolumeType volumeType, FileSizeType fileByteSize, MetaFilePropertySet& prop);
    void _BuildFD2VolumeTypeMap(MetaVolume* volume);
    void _BuildFileKey2VolumeTypeMap(MetaVolume* volume);
    void _CleanUp(void);

    bool nvramMetaVolAvailable;
    bool allVolumesOpened;
    std::map<MetaVolumeType, MetaVolume*> volumeContainer;
    std::unordered_map<FileDescriptorType, MetaVolumeType> fd2VolTypehMap;
    std::unordered_map<StringHashType, MetaVolumeType> fileKey2VolTypeMap;

    void _SetBackupInfo(MetaVolume* volume, MetaLpnType* info);
};
} // namespace pos
