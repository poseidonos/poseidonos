## Define
UT_DEFINE = -DIBOF_CONFIG_USE_MOCK_FS

## Directories
IBOF_DIR = ../../..
DPDK_ROOT_DIR := $(abspath $(IBOF_DIR)/lib/dpdk-stable-19.08.2/include/dpdk)
SPDLOG_SOURCE = spdlog-1.4.2
SPDLOG_ROOT_DIR = $(abspath $(IBOF_DIR)/lib/$(SPDLOG_SOURCE))
UT_INCLUDE = -I$(IBOF_DIR) -I$(IBOF_DIR)/lib -I$(DPDK_ROOT_DIR) -I$(SPDLOG_ROOT_DIR)/include -I$(SPDLOG_ROOT_DIR)/include/spdlog
OBJDIR_REL = ./obj_output
OBJDIR = $(abspath $(OBJDIR_REL))
# Ubuntu 18.04 issue
GTEST_LIB_DIR = /usr/local/lib

## Options
UT_CPPFLAGS = -g -Wall -Wunused -std=c++14 --coverage
UT_LFLAGS = -L$(GTEST_LIB_DIR) -lgmock_main -lpthread

## Execution file name
 # run as sudo
APP0 = ut0_mapper_ut
APP1 = ut1_reversemap_ut
APP2 = ut2_vsamapmanager_ut
APP3 = ut3_sequential_page_finder_ut

## Source files
MAPPER_FULL_SRCS = $(wildcard $(IBOF_DIR)/src/mapper/*.cpp)
MAPPER_EXCLUDE = $(wildcard $(IBOF_DIR)/src/mapper/*_stub.cpp)
MAPPER_EXCLUDE += $(wildcard $(IBOF_DIR)/src/mapper/*_mock.cpp)
MAPPER_SRCS = $(filter-out $(MAPPER_EXCLUDE), $(MAPPER_FULL_SRCS))
# DOC (Depend On Component)
DOC_SRCS = $(IBOF_DIR)/src/mapper/unit_test/lib_test.cpp
DOC_SRCS += $(IBOF_DIR)/src/allocator/allocator_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/allocator/stripe_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/array/array_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/array/array_state_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/array/device/array_device_manager_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/array/meta/array_meta_manager_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/array/partition/partition_manager_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/dump/dump_buffer.cpp
DOC_SRCS += $(IBOF_DIR)/src/dump/dump_manager.cpp
DOC_SRCS += $(IBOF_DIR)/src/dump/dump_shared_ptr.cpp
DOC_SRCS += $(IBOF_DIR)/src/lib/bitmap.cpp
DOC_SRCS += $(IBOF_DIR)/src/logger/logger.cpp
DOC_SRCS += $(IBOF_DIR)/src/logger/filter.cpp
DOC_SRCS += $(IBOF_DIR)/src/logger/preferences.cpp
DOC_SRCS += $(IBOF_DIR)/src/logger/deduplicator.cpp
DOC_SRCS += $(IBOF_DIR)/src/logger/configuration.cpp
DOC_SRCS += $(IBOF_DIR)/src/master_context/config_manager.cpp
DOC_SRCS += $(IBOF_DIR)/src/meta_file_intf/mock_file_intf.cpp
DOC_SRCS += $(IBOF_DIR)/src/meta_file_intf/meta_file_intf.cpp
DOC_SRCS += $(IBOF_DIR)/src/meta_file_intf/async_context.cpp
DOC_SRCS += $(IBOF_DIR)/src/scheduler/event_argument_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/scheduler/event_scheduler_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/scheduler/event_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/state/state_context_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/state/state_manager_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/state/state_publisher_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/sys_event/volume_event_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/volume/volume_list_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/volume/volume_manager_stub.cpp
DOC_SRCS += $(IBOF_DIR)/src/mapper/unit_test/volume_manager_mock.cpp

APP0_SRCS = $(IBOF_DIR)/src/mapper/unit_test/mapper_test.cpp
APP0_SRCS += $(IBOF_DIR)/src/mapper/unit_test/map_io_handler_test.cpp
APP0_SRCS += $(DOC_SRCS)
APP0_MAPPER_EXCLUDE = $(IBOF_DIR)/src/mapper/mapper_address_info.cpp
APP0_MAPPER_EXCLUDE += $(IBOF_DIR)/src/mapper/reverse_map.cpp
APP0_MAPPER_SRCS = $(filter-out $(APP0_MAPPER_EXCLUDE), $(MAPPER_SRCS))
APP0_MAPPER_SRCS += $(IBOF_DIR)/src/mapper/mapper_address_info_mock.cpp
APP0_MAPPER_SRCS += $(IBOF_DIR)/src/mapper/reverse_map_stub.cpp
APP0_SRCS += $(APP0_MAPPER_SRCS)

APP1_SRCS = $(IBOF_DIR)/src/mapper/unit_test/reverse_map_test.cpp
APP1_SRCS += $(DOC_SRCS)
APP1_SRCS += $(MAPPER_SRCS)

APP2_SRCS = $(IBOF_DIR)/src/mapper/unit_test/vsamap_manager_test.cpp
APP2_SRCS += $(DOC_SRCS)
APP2_SRCS += $(MAPPER_SRCS)

APP3_SRCS = $(IBOF_DIR)/src/mapper/unit_test/sequential_page_finder_test.cpp
APP3_SRCS += $(DOC_SRCS)
APP3_SRCS += $(MAPPER_SRCS)

## Object files from source files
APP0_OBJECTS = $(APP0_SRCS:.cpp=.o)
APP1_OBJECTS = $(APP1_SRCS:.cpp=.o)
APP2_OBJECTS = $(APP2_SRCS:.cpp=.o)
APP3_OBJECTS = $(APP3_SRCS:.cpp=.o)

## Commands
ut: configure $(APP0) $(APP1) $(APP2) $(APP3)

configure:
	./$(IBOF_DIR)/configure --with-mockfs

$(APP0): $(OBJDIR) $(APP0_OBJECTS)
	@ccache g++ -o $@ $(UT_INCLUDE) $(UT_CPPFLAGS) $(UT_LFLAGS) $(addprefix $(OBJDIR)/, $(notdir $(APP0_OBJECTS)))

$(APP1): $(OBJDIR) $(APP1_OBJECTS)
	@ccache g++ -o $@ $(UT_INCLUDE) $(UT_CPPFLAGS) $(UT_LFLAGS) $(addprefix $(OBJDIR)/, $(notdir $(APP1_OBJECTS)))

$(APP2): $(OBJDIR) $(APP2_OBJECTS)
	@ccache g++ -o $@ $(UT_INCLUDE) $(UT_CPPFLAGS) $(UT_LFLAGS) $(addprefix $(OBJDIR)/, $(notdir $(APP2_OBJECTS)))

$(APP3): $(OBJDIR) $(APP3_OBJECTS)
	@ccache g++ -o $@ $(UT_INCLUDE) $(UT_CPPFLAGS) $(UT_LFLAGS) $(addprefix $(OBJDIR)/, $(notdir $(APP3_OBJECTS)))

$(OBJDIR):
	mkdir -p $@

# Pattern rules
%.o: %.cpp
	@echo "Compling [$(<F)]..."
	@ccache g++ $(UT_DEFINE) $(UT_INCLUDE) $(UT_CPPFLAGS) -c $< -o $(OBJDIR)/$(notdir $@)

clean:
	rm -rf $(APP0) $(APP1) $(APP2) $(APP3) $(OBJDIR) RevMapWhole *.bin *.txt

reset:
	rm -rf *.bin
