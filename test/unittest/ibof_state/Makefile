ROOT = ../../..
GTEST_DIR = $(ROOT)/lib/googletest-release-1.10.x/googletest
INCLUDE=-I$(ROOT) -I$(ROOT)/lib

TARGET_FILE = $(ROOT)/src/state/ibof_state_manager.cpp \
			  $(ROOT)/src/state/context_list.cpp \
			  $(ROOT)/src/logger/logger.cpp \
			  $(ROOT)/src/state/ibof_state_publisher.cpp 

UT_FILE = tester_main.cpp \
		  ibof_state_tester.cpp
OUTPUT = ibof_state_tester

all:
	g++ ${INCLUDE} $(TARGET_FILE) $(UT_FILE) -lgtest -lgtest_main -lpthread -o $(OUTPUT)
	#g++ ${INCLUDE} $(TARGET_FILE) $(UT_FILE) $(GTEST_DIR)/libgtest.a $(GTEST_DIR)/libgtest_main.a -lpthread -o $(OUTPUT)
clean:
	rm -rf $(OUTPUT)
