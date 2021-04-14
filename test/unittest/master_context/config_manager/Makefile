ROOT = ../../../../
GTEST_DIR = $(ROOT)/lib/googletest-release-1.8.1/googletest

INCLUDE = -I$(ROOT) -I$(ROOT)/lib -I$(ROOT)/src/master_context
TARGET_FILE = $(ROOT)/src/master_context/config_manager.cpp
UT_FILE = config_manager_test.cpp
OUTPUT = config_manager_ut
CFLAGS = -g -lgtest -lpthread 

all:
	g++ $(INCLUDE) $(UT_FILE) $(TARGET_FILE) $(CFLAGS) -o $(OUTPUT)
clean:
	rm -rf $(OUTPUT)
