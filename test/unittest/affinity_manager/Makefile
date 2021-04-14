
ROOT = ../../../
INCLUDE = -I$(ROOT) -I$(ROOT)/lib/
DPDK_ROOT_DIR := $(abspath $(ROOT)/lib/dpdk)
INCLUDE += -I$(DPDK_ROOT_DIR)/include/dpdk
TARGET_FILE = $(ROOT)/src/io/general_io/affinity_manager.cpp\
			  $(ROOT)/src/logger/logger.cpp\
			  $(ROOT)/src/logger/ibof_event_id.cpp\
			  $(ROOT)/src/io/general_io/affinity_viewer.cpp\
			  $(ROOT)/src/master_context/config_manager.cpp

UT_FILE = affinity_viewer_stand_alone.cpp
OUTPUT = affinity_viewer
CFLAGS = -g -lnuma -lspdlog -lpthread
DEFINE += -DSPDLOG_COMPILED_LIB

all:
	g++ $(INCLUDE) $(UT_FILE) $(TARGET_FILE) $(CFLAGS) $(DEFINE) -o $(OUTPUT)
clean:
	rm -rf $(OUTPUT)
