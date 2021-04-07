ROOT = ../../../../
SPDK_INCLUDE := $(ROOT)/lib/spdk-19.10/include/
SPD_LOG := $(ROOT)/lib/spdlog-1.4.2/include/
INCLUDE = -I$(ROOT) -I$(ROOT)/lib/ -I$(SPDK_INCLUDE) -I$(SPD_LOG) -I$(ROOT)/lib/dpdk-stable-19.08.2/include/dpdk/ -I$(ROOT)/tool/library_unit_test/
UT_FILE = mdts_detach_unittest.cpp $(ROOT)/tool/library_unit_test/library_unit_test.cpp
IBOFOS_LIB = $(ROOT)/bin/ibofos_library
all:
	g++ -g -o mdts_detach_test $(INCLUDE) $(UT_FILE) $(IBOFOS_LIB) -lpthread -ltcmalloc
clean:
	rm -rf $(OUTPUT)

