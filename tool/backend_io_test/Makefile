
ROOT = ../../
INCLUDE = -I$(ROOT) -I$(ROOT)/lib/ -I$(SPDK_INCLUDE) -I$(SPD_LOG) -I$(ROOT)/src/network -I$(ROOT)/lib/spdk/include -I$(ROOT)/lib/dpdk/include/dpdk/ -I$(ROOT)/tool/library_unit_test/
INCLUDE += -I$(ROOT)/lib/air/ -I$(ROOT)/lib/air/src/api/
INCLUDE += -I$(ROOT)/src/metafs/include/ 
INCLUDE += -I$(ROOT)/src/metafs/mai/
INCLUDE += -I$(ROOT)/src/metafs/common/
INCLUDE += -I$(ROOT)/src/metafs/mim/
INCLUDE += -I$(ROOT)/src/metafs/lib/
INCLUDE += -I$(ROOT)/src/metafs/log/
INCLUDE += -I$(ROOT)/src/metafs/util/
INCLUDE += -I$(ROOT)/src/metafs/common/
INCLUDE += -I$(ROOT)/src/metafs/config/
INCLUDE += -I$(ROOT)/src/metafs/storage/
INCLUDE += -I$(ROOT)/src/metafs/storage/pstore/
INCLUDE += -I$(ROOT)/src/metafs/mvm/
INCLUDE += -I$(ROOT)/src/metafs/mvm/volume/
INCLUDE += -I$(ROOT)/src/metafs/mvm/volume/nvram/
INCLUDE += -I$(ROOT)/src/metafs/mvm/volume/ssd/
INCLUDE += -I$(ROOT)/src/metafs/msc
INCLUDE += -I$(ROOT)/src/metafs/msc/mbr
SPDLOG_SOURCE := spdlog-1.4.2
SPDLOG_ROOT_DIR = $(abspath $(ROOT)/lib/$(SPDLOG_SOURCE))

INCLUDE += -I$(SPDLOG_ROOT_DIR)/include -I$(SPDLOG_ROOT_DIR)/include/spdlog

IBOF_LDFLAGS += -L$(ROOT)/lib/$(SPDLOG_SOURCE)/lib -lspdlog

UT_FILE = backend_io_test.cpp io_config.cpp $(ROOT)/tool/library_unit_test/library_unit_test.cpp
IBOFOS_LIB = $(ROOT)/bin/ibofos_library
all:
	g++ -g -o backend_io $(INCLUDE) $(UT_FILE) -lpthread -ltcmalloc $(IBOFOS_LIB) -L./lib/air/lib/ $(IBOF_LDFLAGS)
clean:
	rm -rf $(OUTPUT)
