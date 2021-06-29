#include "nvmf_volume_mock.hpp"

namespace pos{

void NvmfVolumeMock::VolumeCreated(struct pos_volume_info* info){
}

void NvmfVolumeMock::VolumeDeleted(struct pos_volume_info* info){
}

void NvmfVolumeMock::VolumeMounted(struct pos_volume_info* info){
}

void NvmfVolumeMock::VolumeUnmounted(struct pos_volume_info* info){
}

void NvmfVolumeMock::VolumeUpdated(struct pos_volume_info* info){
}

NvmfVolumeMock::NvmfVolumeMock() { 
}

NvmfVolumeMock::~NvmfVolumeMock() { 
}

} // namespace pos
