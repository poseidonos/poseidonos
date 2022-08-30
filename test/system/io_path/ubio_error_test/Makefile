
ROOT = ../../../../
INCLUDE = -I$(ROOT) -I$(ROOT)/lib/ -I$(SPDK_INCLUDE) -I$(SPD_LOG) -I$(ROOT)/src/network -I$(ROOT)/lib/spdk/include -I$(ROOT)/lib/dpdk/include/dpdk/ -I$(ROOT)/tool/library_unit_test/ -I$(ROOT)/lib/opentelemetry-cpp/include
UT_FILE = ubio_error_test.cpp $(ROOT)/tool/library_unit_test/library_unit_test.cpp
IBOFOS_LIB = $(ROOT)/bin/ibofos_library
all:
	g++ -g -o ubio_error_test $(INCLUDE) $(UT_FILE) -lpthread -ltcmalloc $(IBOFOS_LIB)
clean:
	rm -rf $(OUTPUT)
