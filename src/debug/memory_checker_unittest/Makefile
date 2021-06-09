#This Test Should be without POS_TRACE_ERROR on memory_checker.cpp
ROOT = ../../../
SPDK_INCLUDE := $(ROOT)/lib/spdk/include/
SPD_LOG := $(ROOT)/lib/spdlog-1.4.2/include/
INCLUDE = -I$(ROOT) -I$(ROOT)/lib/ -I$(SPDK_INCLUDE) -I$(SPD_LOG) -I$(ROOT)/lib/dpdk-stable-19.08.2/include/dpdk/
UT_FILE = memory_checker_unittest.cpp ../memory_checker.cpp  ../../../bin/ibofos_library
all:
	g++ -g -o ./memory_checker_unittest -O0 $(INCLUDE) $(UT_FILE) -lpthread -L$(SPD_LOG)/../lib -lspdlog 
clean:
	rm -rf $(OUTPUT)
