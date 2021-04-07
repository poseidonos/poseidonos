IBOF_SRC_DIR := $(abspath $(CURDIR)/../../../../../)
SPDK_ROOT_DIR := $(IBOF_SRC_DIR)/lib/spdk-19.10

include $(SPDK_ROOT_DIR)/mk/spdk.common.mk
include $(SPDK_ROOT_DIR)/mk/spdk.modules.mk

APP = nvmf_target_test

CXX_SRCS := nvmf_target_test.cpp $(IBOF_SRC_DIR)/src/network/nvmf_target.cpp $(IBOF_SRC_DIR)/src/network/nvmf_target_spdk.cpp $(IBOF_SRC_DIR)/src/network/nvmf_volume.cpp nvmf_volume_mock.cpp $(IBOF_SRC_DIR)/src/device/spdk/spdk.cpp $(IBOF_SRC_DIR)/src/array/logger/array_event.cpp $(IBOF_SRC_DIR)/src/io/affinity_manager.cpp $(IBOF_SRC_DIR)/src/sys_event/volume_event_publisher.cpp  $(IBOF_SRC_DIR)/src/logger/logger.cpp  $(IBOF_SRC_DIR)/src/logger/ibof_event_id.cpp $(IBOF_SRC_DIR)/src/device/ioat_api.cpp

SPDK_LIB_LIST = $(ALL_MODULES_LIST)

ifeq ($(OS),Linux)
ifeq ($(CONFIG_VHOST),y)
SPDK_LIB_LIST += vhost event_vhost
ifeq ($(CONFIG_VHOST_INTERNAL_LIB),y)
SPDK_LIB_LIST += rte_vhost
endif
endif
endif

SPDK_LIB_LIST += event_bdev event_copy event_iscsi event_net event_scsi event_nvmf event_vmd event
SPDK_LIB_LIST += nvmf trace log conf thread util bdev iscsi scsi copy rpc jsonrpc json
SPDK_LIB_LIST += app_rpc log_rpc bdev_rpc net sock notify

ifeq ($(OS),Linux)
SPDK_LIB_LIST += event_nbd nbd
endif

ifeq ($(CONFIG_FC),y)
ifneq ($(strip $(CONFIG_FC_PATH)),)
SYS_LIBS += -L$(CONFIG_FC_PATH)
endif
SYS_LIBS += -lufc
endif

include $(SPDK_ROOT_DIR)/mk/spdk.app.mk

CXXFLAGS += -I$(IBOF_SRC_DIR) -I$(IBOF_SRC_DIR)/src/network -I$(IBOF_SRC_DIR)/lib

all : $(APP)
	@:

$(APP) : $(OBJS) $(SPDK_LIB_FILES) $(COPY_MODULES_FILES) $(SOCK_MODULES_FILES) $(LINKER_MODULES) $(ENV_LIBS)
	$(LINK_CXX)

clean:
	$(CLEAN_C) $(APP)

