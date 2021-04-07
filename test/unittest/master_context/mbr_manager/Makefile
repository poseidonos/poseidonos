ROOT = ../../../../
GTEST_DIR = $(ROOT)/lib/googletest-release-1.8.1/googletest

INCLUDE = -I$(ROOT) -I$(ROOT)/lib -I$(ROOT)/src/master_context -I$(ROOT)/src/device
TARGET_FILE = $(ROOT)/src/master_context/mbr_manager.cpp \
              $(ROOT)/src/cli/json_helper.cpp \
              $(ROOT)/src/device/mock_device.cpp \
              $(ROOT)/src/device/ublock_device.cpp \
              fake_devicemanager.cpp \
              fake_ubio.cpp \
              fake_event_argument.cpp \
              fake_io_dispatcher.cpp \
              fake_io_worker.cpp \
              fake_io_queue.cpp
UT_FILE = mbr_manager_test.cpp
OUTPUT = mbr_manager_ut
CFLAGS = -g -lpthread -lgtest

all:
	g++ $(INCLUDE) $(UT_FILE) $(TARGET_FILE) $(CFLAGS) -o $(OUTPUT)
clean:
	rm -rf $(OUTPUT)
