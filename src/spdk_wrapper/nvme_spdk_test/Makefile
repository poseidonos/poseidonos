ROOT = ../../../../
SPDK_INCLUDE := $(ROOT)/lib/spdk/include/
INCLUDE = -I$(ROOT) -I$(ROOT)/lib/ -I$(SPDK_INCLUDE)
UT_FILE = nvme_spdk_test.cpp
IBOFOS_LIB = $(ROOT)/bin/ibofos_library
all:
	g++ -g -o nvme_spdk_test $(INCLUDE) $(UT_FILE) $(IBOFOS_LIB) -lpthread
#g++ ../../../bin/ibof_bdev_fio_plugin
clean:
	rm -rf $(OUTPUT)

