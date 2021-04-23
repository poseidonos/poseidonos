#pragma once

#include <iostream>
#include <unistd.h>
#include <vector>
#include <memory>
#include <functional>

#include "spdk/stdinc.h"
#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/pos.h"
#include "nvmf_volume.hpp"

namespace pos{

/*
 * NvmfVolumeMock : The class is used by /ibofos/test/testcase/nvmf/target/cpp/NvmfTargetTest.cpp
 * 			to maintain the separate TC without Pos class dependency
 * */
class NvmfVolumeMock final : public NvmfVolume {
public:
	NvmfVolumeMock();
	~NvmfVolumeMock();

	void VolumeCreated(NvmfTarget* target, struct pos_volume_info* info) override;
	void VolumeDeleted(NvmfTarget* target, struct pos_volume_info* info) override;
	void VolumeMounted(NvmfTarget* target, struct pos_volume_info* info) override;
	void VolumeUnmounted(NvmfTarget* target, struct pos_volume_info* info) override;
	void VolumeUpdated(NvmfTarget* target, struct pos_volume_info* info) override;

private:
};

} // namespacae fluidos
