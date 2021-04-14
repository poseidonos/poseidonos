
ROOT = ../../../../
INCLUDE = -I$(ROOT) -I$(ROOT)/lib/
UT_FILE = command_timeout_handler_test.cpp $(ROOT)/tool/library_unit_test/library_unit_test.cpp
IBOFOS_LIB = $(ROOT)/bin/ibofos_library

SPDK_INCLUDE := $(ROOT)/lib/spdk/include/
SPD_LOG := $(ROOT)/lib/spdlog-1.4.2/include/
INCLUDE = -I$(ROOT) -I$(ROOT)/lib/ -I$(SPDK_INCLUDE) -I$(SPD_LOG) -I$(ROOT)/lib/dpdk/include/dpdk/

all:
	g++ -g -o command_timeout_handler_test $(INCLUDE) $(UT_FILE) $(IBOFOS_LIB) -lpthread
clean:
	rm -rf $(OUTPUT)
