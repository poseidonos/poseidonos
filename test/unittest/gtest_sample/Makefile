IBOF_SRC_DIR := $(abspath $(CURDIR)/../../..)
IBOF_NVMF_SRC_DIR := $(IBOF_SRC_DIR)/src/ibofos/network
SPDK_ROOT_DIR := $(IBOF_SRC_DIR)/lib/spdk-19.10
GTEST_DIR := $(IBOF_SRC_DIR)/lib/googletest-release-1.8.1/googletest

include $(SPDK_ROOT_DIR)/mk/spdk.common.mk
include $(SPDK_ROOT_DIR)/mk/spdk.app.mk
include $(SPDK_ROOT_DIR)/mk/spdk.modules.mk

APP = logic_unittest

CXX_SRCS := ./logic.cpp ./logic_unittest.cpp

SPDK_LIB_LIST =  $(ALL_MODULES_LIST)
SPDK_LIB_LIST += event_bdev event_copy event_nvmf event_net
SPDK_LIB_LIST += nvmf event log trace conf thread util bdev copy rpc jsonrpc json net sock
SPDK_LIB_LIST += app_rpc log_rpc trace_rpc bdev_rpc

ifeq ($(OS),Linux)
SPDK_LIB_LIST += event_nbd nbd
endif

CXXFLAGS += -I$(IBOF_SRC_DIR) -I$(IBOF_NVMF_SRC_DIR)
LIBS += -Wl,--whole-archive -L. -lgtest
#LIBS += $(BLOCKDEV_MODULES_LINKER_ARGS) \
#	$(COPY_MODULES_LINKER_ARGS) \
#	$(SOCK_MODULES_LINKER_ARGS) \
#	$(SPDK_LIB_LINKER_ARGS) $(ENV_LINKER_ARGS) \

CXXFLAGS += -isystem $(GTEST_DIR)/include

GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

all : $(APP)
	@:

$(APP) : $(OBJS) libgtest.a $(SPDK_LIB_FILES) $(COPY_MODULES_FILES) $(SOCK_MODULES_FILES) $(LINKER_MODULES) $(ENV_LIBS)
	$(LINK_CXX)
	echo $(LINK_CXX)

gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
	$(GTEST_DIR)/src/gtest-all.cc

libgtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

clean :
	$(CLEAN_C) $(APP)


include $(SPDK_ROOT_DIR)/mk/spdk.deps.mk
