IBOF_SRC_DIR := $(abspath $(CURDIR)/../../../../../)
SPDK_ROOT_DIR := $(IBOF_SRC_DIR)/lib/spdk-19.10
GTEST_DIR := $(IBOF_SRC_DIR)/lib/googletest-release-1.10.x/googletest

include $(SPDK_ROOT_DIR)/mk/spdk.common.mk
include $(SPDK_ROOT_DIR)/mk/spdk.modules.mk

APP = nvmf_target_gtest

CXX_SRCS := nvmf_target_gtest.cpp $(IBOF_SRC_DIR)/src/network/nvmf_target.cpp $(IBOF_SRC_DIR)/src/network/nvmf_target_spdk.cpp $(IBOF_SRC_DIR)/src/network/nvmf_volume.cpp ../cpp/nvmf_volume_mock.cpp $(IBOF_SRC_DIR)/src/device/spdk/spdk.cpp $(IBOF_SRC_DIR)/src/io/affinity_manager.cpp $(IBOF_SRC_DIR)/src/sys_event/volume_event_publisher.cpp $(IBOF_SRC_DIR)/src/logger/logger.cpp  $(IBOF_SRC_DIR)/src/device/ioat_api.cpp  $(IBOF_SRC_DIR)/src/logger/ibof_event_id.cpp

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
LIBS += -Wl,--whole-archive -L. -lgtest

CXXFLAGS += -isystem $(GTEST_DIR)/include

GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

all : $(APP)
	@:

$(APP) : $(OBJS) libgtest.a $(SPDK_LIB_FILES) $(COPY_MODULES_FILES) $(SOCK_MODULES_FILES) $(LINKER_MODULES) $(ENV_LIBS)
	$(LINK_CXX)

gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
	$(GTEST_DIR)/src/gtest-all.cc

libgtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

clean :
	$(CLEAN_C) $(APP)

