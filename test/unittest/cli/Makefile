ROOT = ../../..
GTEST_DIR = $(ROOT)/lib/googletest-release-1.8.1/googletest
INCLUDE=-I$(ROOT) -I$(ROOT)/lib

# export MFS_TOP_DIR=$(ROOT)/src/metafs
# include $(MFS_TOP_DIR)/mfs_include.mk
# INCLUDE += $(MFS_INCLUDE_PATH)

TARGET_FILE = $(ROOT)/src/cli/json_helper.cpp \
			  $(ROOT)/src/cli/cli_server.cpp \
			  $(ROOT)/src/cli/test/request_handler_stub.cpp \
			  $(ROOT)/src/logger/logger.cpp \
			  $(ROOT)/tool/cli_client/test/cli_client_stub.cpp \
			  $(ROOT)/tool/cli_client/command.cpp \
			  $(ROOT)/tool/cli_client/param.cpp \
			  $(ROOT)/tool/cli_client/custom_command.cpp \
			  $(ROOT)/tool/cli_client/command_list.cpp \
			  request.cpp

UT_FILE = tester_main.cpp \
		  cli_tester.cpp
OUTPUT = cli_tester_exe

all:
	g++ ${INCLUDE} $(TARGET_FILE) $(UT_FILE) -lgtest -lgtest_main -lpthread -o $(OUTPUT)
	#g++ ${INCLUDE} $(TARGET_FILE) $(UT_FILE) $(GTEST_DIR)/libgtest.a $(GTEST_DIR)/libgtest_main.a -lpthread -o $(OUTPUT)
clean:
	rm -rf $(OUTPUT)
