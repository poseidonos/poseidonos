## Define
UT_DEFINE = -DIBOF_CONFIG_USE_MOCK_FS

## Directories
IBOF_DIR = ../../..
DPDK_ROOT_DIR := $(abspath $(IBOF_DIR)/lib/dpdk/include/dpdk)
SPDK_ROOT_DIR := $(abspath $(IBOF_DIR)/lib/spdk)
MFS_ROOT := $(abspath $(IBOF_DIR)/src/metafs)
SPDLOG_SOURCE = spdlog-1.4.2
SPDLOG_ROOT_DIR = $(abspath $(IBOF_DIR)/lib/$(SPDLOG_SOURCE))

UT_INCLUDE = -I$(IBOF_DIR) 
UT_INCLUDE += -I$(IBOF_DIR)/lib
UT_INCLUDE += -I$(MFS_ROOT)/include -I$(MFS_ROOT)/mai -I$(MFS_ROOT)/common -I$(MFS_ROOT)/mim -I$(MFS_ROOT)/lib -I$(MFS_ROOT)/log -I$(MFS_ROOT)/util -I$(MFS_ROOT)/config -I$(MFS_ROOT)/storage -I$(MFS_ROOT)/storage/pstore
UT_INCLUDE += -I$(MFS_ROOT)/mvm -I$(MFS_ROOT)/mvm/volume -I$(MFS_ROOT)/mvm/volume/nvram -I$(MFS_ROOT)/mvm/volume/ssd -I$(MFS_ROOT)/msc
UT_INCLUDE += -I$(DPDK_ROOT_DIR)
UT_INCLUDE += -I$(SPDLOG_ROOT_DIR)/include -I$(SPDLOG_ROOT_DIR)/include/spdlog -I$(SPDK_ROOT_DIR)/include

OBJDIR_REL = ./obj_output
OBJDIR = $(abspath $(OBJDIR_REL))
# Ubuntu 18.04 issue
GTEST_LIB_DIR = /usr/local/lib

## Options
UT_CPPFLAGS = -g -Wall -Wunused -std=c++14 --coverage
UT_LFLAGS = -L$(GTEST_LIB_DIR) -lgmock_main -lpthread

## Execution file name
 # run as sudo
APP0 = mapper_ut

## Source files
MAPPER_SUB_DIR= . \
.. \
../map \
../reversemap \
../stripemap \
../vsamap

MAPPER_FULL_SRCS = $(wildcard $(foreach dir, $(MAPPER_SUB_DIR), $(dir)/*.cpp))
MAPPER_EXCLUDE = ../mapper_stub.cpp ../reversemap/reverse_map_stub.cpp
MAPPER_SRCS = $(filter-out $(MAPPER_EXCLUDE), $(MAPPER_FULL_SRCS))
# DOC (Depend On Component)
DOC_SRCS += $(IBOF_DIR)/src/allocator/allocator_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/allocator/block_manager/block_allocator_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/allocator/wb_stripe_manager/stripe_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/array/device/array_device_manager_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/array_mgmt/array_manager_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/array/partition/partition_manager_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/dump/dump_buffer.cpp
DOC_SRCS += $(IBOF_DIR)/src/dump/dump_manager.cpp
DOC_SRCS += $(IBOF_DIR)/src/dump/dump_shared_ptr.cpp
DOC_SRCS += $(IBOF_DIR)/src/helper/json_helper.cpp
DOC_SRCS += $(IBOF_DIR)/src/lib/bitmap.cpp
DOC_SRCS += $(IBOF_DIR)/src/logger/logger.cpp
DOC_SRCS += $(IBOF_DIR)/src/logger/filter.cpp
DOC_SRCS += $(IBOF_DIR)/src/logger/preferences.cpp
DOC_SRCS += $(IBOF_DIR)/src/logger/deduplicator.cpp
DOC_SRCS += $(IBOF_DIR)/src/logger/configuration.cpp
DOC_SRCS += $(IBOF_DIR)/src/master_context/config_manager.cpp
DOC_SRCS += $(IBOF_DIR)/src/master_context/default_configuration.cpp
DOC_SRCS += $(IBOF_DIR)/src/meta_file_intf/mock_file_intf.cpp
DOC_SRCS += $(IBOF_DIR)/src/meta_file_intf/meta_file_intf.cpp
DOC_SRCS += $(IBOF_DIR)/src/meta_file_intf/async_context.cpp
DOC_SRCS += $(IBOF_DIR)/src/event_scheduler/event_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/state/state_context_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/state/state_publisher_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/sys_event/volume_event_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/sys_event/volume_event_publisher_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/volume/volume_list_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/volume/volume_manager_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/volume/volume_service_stub.cpp

APP0_SRCS = $(DOC_SRCS)
APP0_SRCS += $(MAPPER_SRCS)

## Object files from source files
APP0_OBJECTS = $(APP0_SRCS:.cpp=.o)

## Commands
ut: configure $(APP0)

configure:
	./$(IBOF_DIR)/configure --with-mockfs

$(APP0): $(OBJDIR) $(APP0_OBJECTS)
	@ccache g++ -o $@ $(UT_INCLUDE) $(UT_CPPFLAGS) $(UT_LFLAGS) $(addprefix $(OBJDIR)/, $(notdir $(APP0_OBJECTS)))

$(OBJDIR):
	mkdir -p $@

# Pattern rules
%.o: %.cpp
	@echo "Compling [$(<F)]..."
	@ccache g++ $(UT_DEFINE) $(UT_INCLUDE) $(UT_CPPFLAGS) -c $< -o $(OBJDIR)/$(notdir $@)

clean:
	rm -rf $(APP0) $(OBJDIR) RevMapWhole *.bin *.txt

reset:
	rm -rf *.bin
