ROOT = ../../../../
SPDK_INCLUDE := $(ROOT)/lib/spdk/include/
SPD_LOG := $(ROOT)/lib/spdlog-1.4.2/include/
INCLUDE = -I$(ROOT) -I$(ROOT)/lib/ -I$(SPDK_INCLUDE) -I$(SPD_LOG) -I$(ROOT)/lib/dpdk/include/dpdk/ -I$(ROOT)/tool/library_unit_test/ -I$(ROOT)/lib/opentelemetry-cpp/include
UT_FILE = mdts_detach_unittest.cpp $(ROOT)/tool/library_unit_test/library_unit_test.cpp
IBOFOS_LIB = $(ROOT)/bin/ibofos_library
all:
	g++ -g -o mdts_detach_test $(INCLUDE) $(UT_FILE) -lpthread -ltcmalloc $(IBOFOS_LIB)
clean:
	rm -rf $(OUTPUT)

