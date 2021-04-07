#include "nvmf_volume_mock.hpp"

namespace ibofos{

void NvmfVolumeMock::VolumeCreated(struct ibof_volume_info* info){
}

void NvmfVolumeMock::VolumeDeleted(struct ibof_volume_info* info){
}

void NvmfVolumeMock::VolumeMounted(struct ibof_volume_info* info){
}

void NvmfVolumeMock::VolumeUnmounted(struct ibof_volume_info* info){
}

void NvmfVolumeMock::VolumeUpdated(struct ibof_volume_info* info){
}

NvmfVolumeMock::NvmfVolumeMock() { 
}

NvmfVolumeMock::~NvmfVolumeMock() { 
}

} // namespace ibofos
