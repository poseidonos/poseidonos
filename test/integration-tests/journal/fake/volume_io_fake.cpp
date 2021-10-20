#include "src/bio/volume_io.h"
#include "src/include/core_const.h"

namespace pos
{
VolumeIo::VolumeIo(void* buffer, uint32_t unitCount, int arrayId)
: Ubio(buffer, unitCount, arrayId),
  volumeId(UINT32_MAX),
  originCore(INVALID_CORE)
{
}

VolumeIo::VolumeIo(const VolumeIo& volumeIo)
: Ubio(volumeIo),
  volumeId(volumeIo.volumeId),
  originCore(volumeIo.originCore)
{
}

VolumeIo::~VolumeIo(void)
{
}

VolumeIoSmartPtr
VolumeIo::Split(uint32_t sectors, bool tail)
{
    return nullptr;
}

VolumeIoSmartPtr
VolumeIo::GetOriginVolumeIo(void)
{
    VolumeIoSmartPtr originVolumeIo =
        std::dynamic_pointer_cast<VolumeIo>(GetOriginUbio());
    return originVolumeIo;
}

bool
VolumeIo::IsPollingNecessary(void)
{
    return false;
}

uint32_t
VolumeIo::GetVolumeId(void)
{
    return volumeId;
}

void
VolumeIo::SetSectorRba(uint64_t inputSectorRba)
{
    sectorRba = inputSectorRba;
}

uint64_t
VolumeIo::GetSectorRba(void)
{
    return sectorRba;
}

void
VolumeIo::SetVolumeId(uint32_t inputVolumeId)
{
    volumeId = inputVolumeId;
}

uint32_t
VolumeIo::GetOriginCore(void)
{
    return originCore;
}

void
VolumeIo::SetLsidEntry(StripeAddr& inputLsidEntry)
{
    lsidEntry = inputLsidEntry;
}

void
VolumeIo::SetOldLsidEntry(StripeAddr& inputLsidEntry)
{
    oldLsidEntry = inputLsidEntry;
}

const StripeAddr&
VolumeIo::GetLsidEntry(void)
{
    return lsidEntry;
}

const StripeAddr&
VolumeIo::GetOldLsidEntry(void)
{
    return oldLsidEntry;
}

const VirtualBlkAddr&
VolumeIo::GetVsa(void)
{
    return vsa;
}

void
VolumeIo::SetVsa(VirtualBlkAddr& inputVsa)
{
    vsa = inputVsa;
}

bool
VolumeIo::_IsInvalidLsidEntry(StripeAddr& inputLsidEntry)
{
    bool isValid = (inputLsidEntry.stripeId == UNMAP_STRIPE);
    return isValid;
}
} // namespace pos
