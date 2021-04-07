
ROOT = ../../../
INCLUDE = -I$(ROOT) -I$(ROOT)/lib/ 
UT_FILE = largefile_unittest.cpp
IBOFOS_LIB = $(ROOT)/bin/ibofos_library
all:
	g++ -o ./ibofos $(INCLUDE) $(UT_FILE) $(IBOFOS_LIB) -lpthread
	#g++ ../../../bin/ibof_bdev_fio_plugin
clean:
	rm -rf $(OUTPUT)
