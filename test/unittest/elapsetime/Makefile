IBOF_SRC_DIR := $(abspath $(CURDIR)/../../../)
SPDK_ROOT_DIR := $(IBOF_SRC_DIR)/lib/spdk-19.10
include $(SPDK_ROOT_DIR)/mk/spdk.common.mk

APP = elapse_time

CXX_SRCS := elapse_time.cpp

CXXFLAGS += -I$(IBOF_SRC_DIR) -I/$(IBOF_SRC_DIR)/src/lib

all : $(APP)
	@:

$(APP) : $(OBJS)
	$(LINK_CXX)

clean :
	$(CLEAN_C) $(APP)

include $(SPDK_ROOT_DIR)/mk/spdk.deps.mk
